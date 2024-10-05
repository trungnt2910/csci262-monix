#ifndef COMMANDS_UNAME_COMMAND_H_INCLUDED
#define COMMANDS_UNAME_COMMAND_H_INCLUDED

#include <Command.h>

class UnameCommand: public Command
{
public:
    UnameCommand(CommandStartupInfo info);
    virtual int Run() override;
};

#endif // COMMANDS_UNAME_COMMAND_H_INCLUDED
