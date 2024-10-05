#ifndef COMMANDS_LOAD_COMMAND_H_INCLUDED
#define COMMANDS_LOAD_COMMAND_H_INCLUDED

#include <Command.h>

class LoadCommand: public Command
{
public:
    LoadCommand(CommandStartupInfo info);
    virtual int Run() override;
};

#endif // COMMANDS_LOAD_COMMAND_H_INCLUDED
