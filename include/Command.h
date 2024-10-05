#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Console;

struct CommandStartupInfo
{
    std::vector<std::string> args;
    class Console& console;

    CommandStartupInfo(std::vector<std::string> args, class Console& console)
        : args(std::move(args)), console(console) { }
};

class Command
{
private:
    class CommandData* m_data;
protected:
    Command(CommandStartupInfo info);
    virtual ~Command();

    CommandStartupInfo& GetStartupInfo();
public:
    virtual int Run() = 0;
};

using CommandConstructor = std::function<std::shared_ptr<Command>(CommandStartupInfo)>;

struct CommandInfo
{
    std::string name;
    std::string description;
    CommandConstructor constructor;
};

int MxCommandRegister(std::string name, std::string description, CommandConstructor constructor);
int MxCommandRun(CommandStartupInfo info);
int MxCommandList(std::vector<CommandInfo>& commands);

#endif // COMMAND_H_INCLUDED
