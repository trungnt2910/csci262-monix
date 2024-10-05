#include "Command.h"

#include <map>
#include <mutex>
#include <string>

#include <Console.h>
#include <Logger.h>

#include "AsyncPatchProtection.h"

class CommandData
{
public:
    CommandStartupInfo startupInfo;
};

Command::Command(CommandStartupInfo info)
    : m_data(new CommandData{ .startupInfo = std::move(info)})
{
}

Command::~Command()
{
    delete m_data;
}

CommandStartupInfo& Command::GetStartupInfo()
{
    return m_data->startupInfo;
}

static std::mutex s_CommandsLock;
static std::map<std::string, CommandInfo> s_CommandDatabase;

int MxCommandRegister(std::string name, std::string description, CommandConstructor constructor)
{
    MxAsyncPatchProtectionTrigger();

    std::lock_guard lock(s_CommandsLock);

    auto [it, inserted] = s_CommandDatabase.try_emplace(
        name,
        CommandInfo
        {
            .name = name,
            .description = std::move(description),
            .constructor = std::move(constructor)
        }
    );

    if (!inserted)
    {
        Logger.LogError("A command with the name ", name, " already exists.");
        errno = EEXIST;
        return -1;
    }

    return 0;
}

int MxCommandRun(CommandStartupInfo info)
{
    if (info.args.size() == 0)
    {
        Logger.LogWarning("Got an empty command.");
        errno = EINVAL;
        return -1;
    }

    std::shared_ptr<Command> commandToRun;

    {
        std::lock_guard lock(s_CommandsLock);
        auto it = s_CommandDatabase.find(info.args.front());

        if (it == s_CommandDatabase.end())
        {
            Logger.LogWarning("Command \"", info.args.front(), "\" not found.");
            errno = ENOENT;

            info.console.GetOutput() << "Command \"" << info.args.front() << "\" not found."
                                     << std::endl;

            return -1;
        }

        commandToRun = it->second.constructor(std::move(info));
    }

    int returnValue = commandToRun->Run();

    if (returnValue != 0)
    {
        errno = returnValue;
        return -1;
    }

    return 0;
}

int MxCommandList(std::vector<CommandInfo>& commands)
{
    std::lock_guard lock(s_CommandsLock);

    commands.clear();
    commands.reserve(s_CommandDatabase.size());

    for (auto& [name, entry]: s_CommandDatabase)
    {
        commands.push_back(entry);
    }

    return 0;
}
