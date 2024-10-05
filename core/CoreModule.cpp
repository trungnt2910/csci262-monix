#include "Module.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <string>

#include <dlfcn.h>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <Logger.h>

#include "AsyncPatchProtection.h"
#include "ProtectedRegion.h"

struct ExtendedModuleInfo: public ModuleInfo
{
    void* handle;
};

static std::mutex s_ModulesLock;
static std::map<std::string, ExtendedModuleInfo> s_ModuleDatabase;

static int s_RegionIdModuleTrustedPublishers = -1;
static int s_RegionIdModulePassword = -1;

int MxInitializeModule()
{
    Logger.LogTrace("Initializing module");

    // TRUSTED PUBLISHERS

    constexpr char publishers[] =
        "Trung Nguyen\0"
        "trungnt2910\0"
        "ttn903\0"
        "Project Reality\0"
        "AzureAms Programming Club\0"
        "Microsoft Corporation\0"
        "\0";
    s_RegionIdModuleTrustedPublishers = MxProtectedRegionAdd("ModuleTrustedPublishers",
        std::span<const char>(publishers, sizeof(publishers) - 1));

    if (s_RegionIdModuleTrustedPublishers == -1)
    {
        Logger.LogError("Failed to allocate protected region for module publishers: ",
            strerror(errno), '.');
        return -1;
    }

    // PASSWORD

    // JustM0nika!, SHA-3 hash.
    constexpr std::string_view password(
        "d2bbd8e770810d157a8ae766b7d72a31692f27202f4ed57a7a05dbe2897b59eb"
        "1e05f672d295d1c104754225c9c2fcc623398c4ce6d1af2d70d810fba6dc7c4f"
    );
    s_RegionIdModulePassword = MxProtectedRegionAdd("ModulePassword",
        std::span<const char>(password.begin(), password.size()));

    if (s_RegionIdModulePassword == -1)
    {
        Logger.LogError("Failed to allocate protected region for module password: ",
            strerror(errno), '.');
        return -1;
    }

    return 0;
}

static bool MxpModuleIsTrustedPublisher(const ModuleInfo& info)
{
    std::span<const char> storedPublishers;
    if (MxProtectedRegionGet(s_RegionIdModuleTrustedPublishers, storedPublishers) == -1)
    {
        Logger.LogError("Failed to retrieve stored publishers: ", strerror(errno), '.');
        return false;
    }

    auto it = storedPublishers.begin();
    while (it != storedPublishers.end())
    {
        std::string_view currentPublisher(&*it);

        Logger.LogTrace("Current publisher: ", currentPublisher, '.');

        if (currentPublisher == info.publisher)
        {
            return true;
        }

        it += currentPublisher.size() + 1;
    }

    Logger.LogError("Unknown publisher: ", info.publisher, '.');
    return false;
}

static bool MxpModuleIsPasswordCorrect(const std::string& password)
{
    uint32_t digest_length = SHA512_DIGEST_LENGTH;
    const EVP_MD* algorithm = EVP_sha3_512();
    uint8_t* digest = (uint8_t*)OPENSSL_malloc(digest_length);
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, algorithm, nullptr);
    EVP_DigestUpdate(context, password.c_str(), password.size());
    EVP_DigestFinal_ex(context, digest, &digest_length);
    EVP_MD_CTX_destroy(context);

    std::stringstream hashStream;
    for (uint32_t i = 0; i < digest_length; ++i)
    {
        hashStream << std::setw(2) << std::setfill('0') << std::hex << (int)digest[i];
    }
    std::string hash = hashStream.str();

    OPENSSL_free(digest);

    std::span<const char> storedHash;
    if (MxProtectedRegionGet(s_RegionIdModulePassword, storedHash) == -1)
    {
        Logger.LogError("Failed to retrieve stored password: ", strerror(errno), '.');
        return false;
    }

    return (hash.size() == storedHash.size())
        && memcmp(hash.c_str(), storedHash.data(), hash.size()) == 0;
}

int MxModuleLoad(const std::string& name, const std::string& password)
{
    MxAsyncPatchProtectionTrigger();

    if (!MxpModuleIsPasswordCorrect(password))
    {
        Logger.LogError("User does not have the permissions to install modules.");
        errno = EPERM;
        return -1;
    }

    std::lock_guard lock(s_ModulesLock);

    if (s_ModuleDatabase.contains(name))
    {
        Logger.LogError("A module with the name ", name, " is already loaded.");
        errno = EEXIST;
        return -1;
    }

    std::filesystem::path executablePath = std::filesystem::canonical("/proc/self/exe");
    std::filesystem::path executableDir = executablePath.parent_path();
    std::filesystem::path modulePath = executableDir / ("lib" + name + ".so");

    void* handle = dlopen(modulePath.c_str(), RTLD_LOCAL | RTLD_NOW);
    if (handle == nullptr)
    {
        Logger.LogError("dlopen for module ", name.c_str(), " failed: ", dlerror(), '.');
        errno = ENOENT;
        return -1;
    }

    const ModuleInfo* moduleInfo = (ModuleInfo*)dlsym(handle, MX_MODULE_INFO_SYMBOL_NAME);
    if (moduleInfo == nullptr)
    {
        Logger.LogError("dlsym for module info symbol failed: ", dlerror(), '.');
        errno = EINVAL;
        return -1;
    }

    if (!MxpModuleIsTrustedPublisher(*moduleInfo))
    {
        Logger.LogError("Module from untrusted publisher ", moduleInfo->publisher, '.');
        errno = EPERM;
        return -1;
    }

    if (moduleInfo->init() == -1)
    {
        Logger.LogError("Module initialization failed.");
        dlclose(handle);
        return -1;
    }

    s_ModuleDatabase.try_emplace(name, ExtendedModuleInfo
    {
        { *moduleInfo },
        /* .handle = */handle
    });

    Logger.LogInfo("Installed module ", moduleInfo->name,
        " from publisher ", moduleInfo->publisher,
        " as ", name, ".");

    return 0;
}
