#ifndef COMMANDS_HELP_COMMAND_H_INCLUDED
#define COMMANDS_HELP_COMMAND_H_INCLUDED

#include <Command.h>

class HelpCommand: public Command
{
public:
    HelpCommand(CommandStartupInfo info);
    virtual int Run() override;
};

#endif // COMMANDS_HELP_COMMAND_H_INCLUDED
