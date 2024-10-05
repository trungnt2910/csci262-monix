#ifndef COMMANDS_EXIT_COMMAND_H_INCLUDED
#define COMMANDS_EXIT_COMMAND_H_INCLUDED

#include <Command.h>

class ExitCommand: public Command
{
public:
    ExitCommand(CommandStartupInfo info);
    virtual int Run() override;
};

#endif // COMMANDS_EXIT_COMMAND_H_INCLUDED
