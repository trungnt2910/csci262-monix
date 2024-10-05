#include "UnameCommand.h"

#include <iostream>
#include <string>

#include <Console.h>
#include <System.h>
#include <Util.h>

UnameCommand::UnameCommand(CommandStartupInfo info)
    : Command(std::move(info))
{
    // No-op.
}

int UnameCommand::Run()
{
    std::vector<std::string>& args = GetStartupInfo().args;

    bool all = args.size() > 1 &&
        (std::find(args.begin() + 1, args.end(), "-a") != args.end());

    std::string kernelName;
    MX_RETURN_IF_FAIL(MxSystemGetKernelName(kernelName));

    if (!all)
    {
        GetStartupInfo().console.GetOutput()
            << kernelName << std::endl;
        return 0;
    }

    std::string hostName;
    std::string kernelRelease;
    std::string kernelVersion;
    std::string machine;
    std::string os;

    MX_RETURN_IF_FAIL(MxSystemGetHostName(hostName));
    MX_RETURN_IF_FAIL(MxSystemGetKernelRelease(kernelRelease));
    MX_RETURN_IF_FAIL(MxSystemGetKernelVersion(kernelVersion));
    MX_RETURN_IF_FAIL(MxSystemGetMachine(machine));
    MX_RETURN_IF_FAIL(MxSystemGetOperatingSystem(os));

    GetStartupInfo().console.GetOutput()
        << kernelName << " " << hostName << " " << kernelRelease << " " << kernelVersion << " "
        << machine << " " << os << std::endl;
    return 0;
}
