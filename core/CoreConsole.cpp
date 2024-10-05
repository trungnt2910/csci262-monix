#include "CoreConsole.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <vector>

#include <Command.h>
#include <Logger.h>
#include <System.h>
#include <Util.h>

#include "Signal.h"

static std::mutex s_ConsolesLock;
static std::set<class Console*> s_ActiveConsoles;

class ConsoleData
{
public:
    std::istream* input;
    std::ostream* output;
    std::string prompt;
    bool interrupted = false;
    bool newPrompt = false;
    bool exited = false;
};

Console::Console(std::istream& input, std::ostream& output)
{
    m_data = new ConsoleData();

    m_data->input = &input;
    m_data->output = &output;

    {
        std::lock_guard lock(s_ConsolesLock);
        s_ActiveConsoles.insert(this);
    }
}

Console::~Console()
{
    if (!m_data->exited)
    {
        Exit();
    }

    delete m_data;
}

void Console::Exit()
{
    m_data->exited = true;
    std::lock_guard lock(s_ConsolesLock);
    s_ActiveConsoles.erase(this);
}

std::istream& Console::GetInput()
{
    return *m_data->input;
}

std::ostream& Console::GetOutput()
{
    return *m_data->output;
}

int Console::Run()
{
    std::string user;
    std::string host;

    std::string line;
    std::stringstream lineStream;

    std::string arg;
    std::vector<std::string> argsVector;

    while (true)
    {
        try
        {
            if (MxSystemGetCurrentUser(user) == -1)
            {
                Logger.LogWarning("Failed to get the current username: ", strerror(errno));
                user = "nobody";
            }

            if (MxSystemGetHostName(host) == -1)
            {
                Logger.LogWarning("Failed to get the current hostname: ", strerror(errno));
                host = "nowhere";
            }

            m_data->prompt = user + "@" + host + "> ";

            do
            {
                // If we need a new prompt, or if this is our first attempt.
                if (m_data->newPrompt || !m_data->interrupted)
                {
                    GetOutput() << m_data->prompt << std::flush;
                }

                m_data->interrupted = false;
                m_data->newPrompt = false;

                line.clear();
                std::getline(GetInput(), line);

                if (m_data->interrupted)
                {
                    GetInput().clear();
                }
            }
            while (m_data->interrupted);

            lineStream.str(std::move(line));
            lineStream.clear();

            argsVector.clear();
            while (lineStream)
            {
                arg.clear();
                lineStream >> arg;

                if (arg.empty())
                {
                    break;
                }

                argsVector.push_back(arg);
            }

            if (MxCommandRun(CommandStartupInfo(std::move(argsVector), *this)) == -1)
            {
                Logger.LogError("Failed to execute command: ", strerror(errno));
            }

            GetOutput() << std::flush;
        }
        catch (const ExitException &exit)
        {
            Logger.LogInfo("Console exit requested, code: ", exit.GetStatusCode(), ".");
            Exit();
            return exit.GetStatusCode();
        }
        catch (const std::exception &e)
        {
            Logger.LogError("Execution failed with an unhandled exception: ", e.what());
        }
        catch (...)
        {
            Logger.LogError("Execution failed with an unhandled exception.");
        }
    }
}

void Console::Interrupt(bool newPrompt)
{
    if (newPrompt)
    {
        GetOutput().put('\n');
    }
    std::flush(GetOutput());

    m_data->interrupted = true;
    m_data->newPrompt = m_data->newPrompt || newPrompt;
}

class ConsoleImpl: public Console
{
public:
    ConsoleImpl()
        : Console(std::cin, std::cout)
    {
    }
};

static ConsoleImpl s_Implementation;
class Console& Console = s_Implementation;

int MxInitializeConsole()
{
    Logger.LogTrace("Initializing console");

    MxSignalHandlerAdd(-1, [](int)
    {
        // Add a soft interrupt for the default console.
        // When a signal occurs, reading from STDIN will be interrupted.
        // We have to reset this state to prevent further errors.
        Console.Interrupt(/* newPrompt =*/false);
    });

    MX_RETURN_IF_FAIL(atexit([]()
    {
        std::lock_guard lock(s_ConsolesLock);
        for (class Console* console: s_ActiveConsoles)
        {
            console->GetOutput()
                << "Monix will shut down NOW." << std::endl;
        }
    }));

    return 0;
}
