#ifndef COMMANDS_SHUTDOWN_COMMAND_H_INCLUDED
#define COMMANDS_SHUTDOWN_COMMAND_H_INCLUDED

#include <Command.h>

class ShutdownCommand: public Command
{
public:
    ShutdownCommand(CommandStartupInfo info);
    virtual int Run() override;
};

#endif // COMMANDS_SHUTDOWN_COMMAND_H_INCLUDED
