#include "AsyncPatchProtection.h"

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <semaphore>
#include <thread>
#include <vector>

#include <signal.h>
#include <sys/mman.h>
#include <time.h>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <Logger.h>
#include <Util.h>

#include "CoreUtils.h"
#include "ProtectedRegion.h"
#include "Settings.h"
#include "Signal.h"

static std::map<std::string, std::vector<char>> s_Hashes;
static std::unique_ptr<std::thread> s_BackgroundThread;
static std::counting_semaphore<> s_CallbackSemaphore(0);
static timer_t s_BackgroundTimer;
static std::chrono::nanoseconds s_CallbackTotalTime;
static bool s_ExitRequested = false;

constexpr int MX_ASYNC_PATCH_PROTECTION_INTERVAL_SECONDS = 10;

static void MxpAsyncPatchProtectionLoop();
static int MxpAsyncPatchProtectionCallback();
static void MxpAsyncPatchProtectionCleanup();
// memcpy but bypasses page protection.
static int MxpGodMemcpy(const void* dst, const void* src, size_t size);
static std::vector<char> MxpHashSha3_512(const std::span<const char>& data);

MX_HIDDEN_FROM_MODULES
int MxInitializeAsyncPatchProtection()
{
    Logger.LogTrace("Initializing patch protection");
    return 0;
}

MX_HIDDEN_FROM_MODULES
int MxAsyncPatchProtectionStart()
{
    if (g_OptionUnguarded)
    {
        Logger.LogInfo("System running in unguarded mode.");
        return 0;
    }

    Logger.LogTrace("Locking down protected regions.");
    if (mprotect(g_ProtectedPages, MX_PROTECTED_REGION_SIZE, PROT_READ) == -1)
    {
        Logger.LogError("Failed to protect protected region pages.");
        return -1;
    }

    Logger.LogTrace("Calculating hashes for protected regions.");
    for (size_t i = 0; i < g_ProtectedInfoCount; ++i)
    {
        auto [it, inserted] = s_Hashes.try_emplace(
            g_ProtectedInfo[i].name,
            MxpHashSha3_512(
                std::span<const char>(
                    g_ProtectedInfo[i].data,
                    g_ProtectedInfo[i].size
                )
            )
        );

        if (!inserted)
        {
            errno = EINVAL;
            Logger.LogError("Protected region headers are invalid!");
        }
    }

    Logger.LogTrace("Spawning background thread.");
    s_BackgroundThread = std::make_unique<std::thread>(MxpAsyncPatchProtectionLoop);

    Logger.LogTrace("Installing signal handlers.");
    MX_RETURN_IF_FAIL(MxSignalHandlerAdd(MX_ALL_SIGNALS, [](int)
    {
        MxAsyncPatchProtectionTrigger();
    }));

    Logger.LogTrace("Creating patch protection interval timer.");
    MX_RETURN_IF_FAIL(timer_create(CLOCK_REALTIME, nullptr, &s_BackgroundTimer));

    Logger.LogTrace("Arming patch protection interval timer.");
    itimerspec timerspec;
    timerspec.it_value.tv_nsec = 0;
    timerspec.it_value.tv_sec = MX_ASYNC_PATCH_PROTECTION_INTERVAL_SECONDS;
    timerspec.it_interval.tv_nsec = 0;
    timerspec.it_interval.tv_sec = MX_ASYNC_PATCH_PROTECTION_INTERVAL_SECONDS;
    MX_RETURN_IF_FAIL(timer_settime(s_BackgroundTimer, 0, &timerspec, nullptr));

    MX_RETURN_IF_FAIL(atexit(MxpAsyncPatchProtectionCleanup));

    Logger.LogTrace("Patch protection started.");

    return 0;
}

MX_HIDDEN_FROM_MODULES
int MxAsyncPatchProtectionTrigger()
{
    if (s_BackgroundThread == nullptr)
    {
        errno = EINVAL;
        return -1;
    }

    s_CallbackSemaphore.release();

    return 0;
}

static void MxpAsyncPatchProtectionLoop()
{
    while (!s_ExitRequested)
    {
        s_CallbackSemaphore.acquire();
        if (MxpAsyncPatchProtectionCallback() == -1)
        {
            Logger.LogError("Monix system integrity check failed.");
            abort();
        }
    }
}

static int MxpAsyncPatchProtectionCallback()
{
    Logger.LogTrace("Patch protection callback called.");

    std::chrono::time_point callbackStart =
        std::chrono::high_resolution_clock::now();

    if (s_Hashes.size() != g_ProtectedInfoCount)
    {
        // Incorrect number of protected regions.
        Logger.LogError("Incorrect number of protected regions.");
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < g_ProtectedInfoCount; ++i)
    {
        std::string name = g_ProtectedInfo[i].name;
        std::vector<char> hash = MxpHashSha3_512(
            std::span<const char>(
                g_ProtectedInfo[i].data,
                g_ProtectedInfo[i].size
            )
        );

        auto it = s_Hashes.find(name);
        if (it == s_Hashes.end())
        {
            Logger.LogError("Corrupted protected region name: ", name, ".");
            errno = EINVAL;
            return -1;
        }

        if (hash != it->second)
        {
            Logger.LogWarning("Corrupted data for protected region: ", name, ".");

            // Try to recover some harmless patches.
            if (name == "KernelName")
            {
                const char correctData[8] = "Monix";
                const size_t correctSize = sizeof(correctData);
                MX_RETURN_IF_FAIL(MxpGodMemcpy(
                    &g_ProtectedInfo[i].size, &correctSize, sizeof(correctSize)));
                MX_RETURN_IF_FAIL(MxpGodMemcpy(g_ProtectedInfo[i].data, correctData, correctSize));
            }
            else if (name == "ProtectedRegionMagic")
            {
                const int correctData = MX_PROTECTED_REGION_MAGIC;
                const size_t correctSize = sizeof(correctData);
                MX_RETURN_IF_FAIL(MxpGodMemcpy(
                    &g_ProtectedInfo[i].size, &correctSize, sizeof(correctSize)));
                MX_RETURN_IF_FAIL(MxpGodMemcpy(g_ProtectedInfo[i].data, &correctData, correctSize));
            }
            else
            {
                // Otherwise, fail.
                Logger.LogError("Corrupted region is not recoverable: ", name, ".");
                return -1;
            }
        }
    }

    std::chrono::time_point callbackEnd =
        std::chrono::high_resolution_clock::now();

    s_CallbackTotalTime += callbackEnd - callbackStart;

    Logger.LogTrace("Patch protection has consumed ",
        s_CallbackTotalTime.count() / 1000000.0,
        "ms of processing time.");

    return 0;
}

static void MxpAsyncPatchProtectionCleanup()
{
    s_ExitRequested = true;
    MxAsyncPatchProtectionTrigger();
    s_BackgroundThread->join();
    s_BackgroundThread.release();
}

static int MxpGodMemcpy(const void* dst, const void* src, size_t size)
{
    intptr_t begin = (intptr_t)((char*)dst);
    intptr_t end = (intptr_t)((char*)dst + size);

    size_t pageSize = sysconf(_SC_PAGE_SIZE);
    begin = begin & (~(pageSize - 1));
    end = (end + pageSize - 1) & (~(pageSize - 1));

    size_t protectSize = end - begin;

    MX_RETURN_IF_FAIL(mprotect((void*)begin, protectSize, PROT_READ | PROT_WRITE));

    memcpy((void*)dst, src, size);

    MX_RETURN_IF_FAIL(mprotect((void*)begin, protectSize, PROT_READ));

    return 0;
}

static std::vector<char> MxpHashSha3_512(const std::span<const char>& data)
{
    std::vector<char> result;

    uint32_t digest_length = SHA512_DIGEST_LENGTH;
    const EVP_MD* algorithm = EVP_sha3_512();
    uint8_t* digest = (uint8_t*)OPENSSL_malloc(digest_length);
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, algorithm, nullptr);
    EVP_DigestUpdate(context, data.data(), data.size_bytes());
    EVP_DigestFinal_ex(context, digest, &digest_length);
    EVP_MD_CTX_destroy(context);

    result.reserve(digest_length);
    for (uint32_t i = 0; i < digest_length; ++i)
    {
        result.push_back(digest[i]);
    }

    OPENSSL_free(digest);

    return result;
}
