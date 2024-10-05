#include "CoreSystem.h"

#include <cstring>
#include <iostream>
#include <span>

#include <limits.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <Logger.h>
#include <Util.h>

#include "ProtectedRegion.h"

static int s_RegionIdKernelName = -1;

int MxInitializeSystem()
{
    Logger.LogTrace("Initializing system");

    constexpr std::string_view originalKernelName(
        "Monix\0\0\0", 8  // add some padding to make 8 bytes.
    );
    s_RegionIdKernelName = MxProtectedRegionAdd("KernelName",
        std::span<const char>(originalKernelName.begin(), originalKernelName.size()));

    if (s_RegionIdKernelName == -1)
    {
        Logger.LogError("Failed to allocate protected region for kernel name: ",
            strerror(errno), '.');
        return -1;
    }

    std::string kernelName;
    std::string kernelRelease;
    std::string kernelVersion;
    MX_RETURN_IF_FAIL(MxSystemGetKernelName(kernelName));
    MX_RETURN_IF_FAIL(MxSystemGetKernelRelease(kernelRelease));
    MX_RETURN_IF_FAIL(MxSystemGetKernelVersion(kernelVersion));

    Logger.LogInfo("================================================================");
    Logger.LogInfo(kernelName, " version ", kernelRelease, " (compiled ", kernelVersion, ")");
    Logger.LogInfo("Copyright <C> 2024 Group 1");
    Logger.LogInfo("    Trung Nguyen (ttn903)");
    Logger.LogInfo("    Viet Thai Nguyen (tvn199)");
    Logger.LogInfo("    Eden Sun (zs754)");
    Logger.LogInfo("    Angie Tran (ngt679)");
    Logger.LogInfo("================================================================");
    Logger.LogInfo("Check out lxmonika & Windows Subsystem for Monix:");
    Logger.LogInfo("https://go.trungnt2910.com/monika");
    Logger.LogInfo("================================================================");
    Logger.LogInfo("Monix is a part of Project Reality.");
    Logger.LogInfo("https://reality.trungnt2910.com/discord");
    Logger.LogInfo("================================================================");

    return 0;
}

int MxSystemGetCurrentUser(std::string& user)
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw != nullptr)
    {
        user = pw->pw_name;
        return 0;
    }
    return -1;
}

int MxSystemGetHostName(std::string& host)
{
    host.resize(HOST_NAME_MAX);
    if (gethostname(host.data(), host.size()) == 0)
    {
        host.resize(strlen(host.c_str()));
        return 0;
    }
    host.clear();
    return -1;
}

int MxSystemGetKernelName(std::string& name)
{
    std::span<const char> kernelNameSpan;
    if (MxProtectedRegionGet(s_RegionIdKernelName, kernelNameSpan) == 0)
    {
        name.reserve(kernelNameSpan.size());
        name = kernelNameSpan.data();
        return 0;
    }
    name.clear();
    return -1;
}

int MxSystemGetKernelRelease(std::string& release)
{
    release = "0.0.1-CSCI262-A3";
    return 0;
}

int MxSystemGetKernelVersion(std::string& version)
{
    version = __DATE__  " " __TIME__;
    return 0;
}

int MxSystemGetMachine(std::string& machine)
{
    utsname name;
    if (uname(&name) == 0)
    {
        machine = name.machine;
        return 0;
    }
    machine.clear();
    return -1;
}

int MxSystemGetOperatingSystem(std::string& os)
{
    os = "CSCI262/Monix";
    return 0;
}
