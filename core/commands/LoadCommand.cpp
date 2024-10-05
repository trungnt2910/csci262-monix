#include "LoadCommand.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

#include <signal.h>
#include <unistd.h>

#include <Console.h>
#include <Module.h>
#include <Util.h>

LoadCommand::LoadCommand(CommandStartupInfo info)
    : Command(std::move(info))
{
    // No-op.
}

int LoadCommand::Run()
{
    std::vector<std::string>& args = GetStartupInfo().args;

    if (args.size() < 2)
    {
        GetStartupInfo().console.GetOutput()
            << "Usage: " << args[0] << " <module name>" << std::endl;

        errno = EINVAL;
        return -1;
    }

    std::string password;

    // For the default console, try to use getpass.
    if (&GetStartupInfo().console == &Console)
    {
        sigset_t set;
        sigfillset(&set);
        sigset_t oldSet;
        MX_RETURN_IF_FAIL(pthread_sigmask(SIG_BLOCK, &set, &oldSet));
        password = getpass("password: ");
        MX_RETURN_IF_FAIL(pthread_sigmask(SIG_SETMASK, &oldSet, nullptr));
    }
    else
    {
        GetStartupInfo().console.GetOutput()
            << "password: " << std::flush;
        std::getline(GetStartupInfo().console.GetInput(), password);
    }

    if (MxModuleLoad(args[1], password) == -1)
    {
        GetStartupInfo().console.GetOutput()
            << "Failed to load the \"" << args[1] << "\" module: "
                << strerror(errno) << '.' << std::endl;
        return -1;
    }

    return 0;
}
