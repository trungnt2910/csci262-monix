#include "Signal.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>

#include <signal.h>

#include <Logger.h>

std::map<int, std::vector<std::function<void(int)>>> s_Handlers;
std::map<int, struct sigaction> s_OldActions;
std::mutex s_SignalsLock;

static void MxpSignalTrampoline(int sig)
{
    std::lock_guard lock(s_SignalsLock);

    if (s_Handlers.contains(sig))
    {
        for (const auto& handler: s_Handlers[sig])
        {
            handler(sig);
        }
    }

    if (s_Handlers.contains(-1))
    {
        for (const auto& handler: s_Handlers[-1])
        {
            handler(sig);
        }
    }
}

int MxInitializeSignal()
{
    Logger.LogTrace("Initializing signals");
    return 0;
}

int MxSignalHandlerAdd(int sig, std::function<void(int)> handler)
{
    if ((sig != MX_ALL_SIGNALS) && (sig <= 0 || sig > SIGRTMAX))
    {
        errno = EINVAL;
        return -1;
    }

    std::lock_guard lock(s_SignalsLock);

    if (!s_Handlers.contains(sig))
    {
        struct sigaction newAction;
        memset(&newAction, 0, sizeof(newAction));
        newAction.sa_handler = MxpSignalTrampoline;

        if (sig != MX_ALL_SIGNALS)
        {
            if (sigaction(sig, &newAction, &s_OldActions[sig]) == -1)
            {
                s_OldActions.erase(sig);
                return -1;
            }
        }
        else
        {
            for (int currentSignal = 1; currentSignal <= SIGRTMAX; ++currentSignal)
            {
                if (s_OldActions.contains(currentSignal))
                {
                    continue;
                }

                struct sigaction oldAction;
                if (sigaction(currentSignal, &newAction, &oldAction) == -1)
                {
                    Logger.LogWarning("Failed to install signal handler for signal ", currentSignal,
                        ": ", strerror(errno), ".");
                    continue;
                }

                s_OldActions.emplace(currentSignal, oldAction);
            }
        }
    }

    s_Handlers[sig].push_back(std::move(handler));
    return 0;
}
