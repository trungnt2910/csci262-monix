#include "HelpCommand.h"

#include <iomanip>
#include <iostream>
#include <string>

#include <Console.h>
#include <System.h>
#include <Util.h>

HelpCommand::HelpCommand(CommandStartupInfo info)
    : Command(std::move(info))
{
    // No-op.
}

int HelpCommand::Run()
{
    std::string kernelName;
    std::string kernelRelease;
    std::string kernelVersion;
    MX_RETURN_IF_FAIL(MxSystemGetKernelName(kernelName));
    MX_RETURN_IF_FAIL(MxSystemGetKernelRelease(kernelRelease));
    MX_RETURN_IF_FAIL(MxSystemGetKernelVersion(kernelVersion));

    GetStartupInfo().console.GetOutput()
        << kernelName << " version " << kernelRelease
            << " (compiled " << kernelVersion << ")" << std::endl
        << "Copyright <C> 2024 Trung Nguyen (ttn903) & Group 1" << std::endl
        << std::endl;

    std::vector<CommandInfo> commands;
    MX_RETURN_IF_FAIL(MxCommandList(commands));

    size_t commandNameWidth = 0;
    for (const auto& entry: commands)
    {
        commandNameWidth = std::max(commandNameWidth, entry.name.size());
    }
    commandNameWidth += 2;

    for (const auto& entry: commands)
    {
        GetStartupInfo().console.GetOutput()
            << std::left << std::setw(commandNameWidth) << entry.name
            << entry.description
            << std::endl;
    }

    return 0;
}
