#include "ExitCommand.h"

#include <string>

#include <Console.h>

ExitCommand::ExitCommand(CommandStartupInfo info)
    : Command(std::move(info))
{
    // No-op.
}

int ExitCommand::Run()
{
    int status = 0;

    if (GetStartupInfo().args.size() >= 2)
    {
        status = std::stoi(GetStartupInfo().args[1]);
    }

    throw ExitException(status);
}
