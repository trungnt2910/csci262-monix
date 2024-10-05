#include "ShutdownCommand.h"

#include <iostream>
#include <string>

#include <Console.h>

ShutdownCommand::ShutdownCommand(CommandStartupInfo info)
    : Command(std::move(info))
{
    // No-op.
}

int ShutdownCommand::Run()
{
    int status = 0;

    if (GetStartupInfo().args.size() >= 2)
    {
        status = std::stoi(GetStartupInfo().args[1]);
    }

    GetStartupInfo().console.GetOutput()
        << "Monix is shutting down..."
        << std::endl;

    exit(status);
}
