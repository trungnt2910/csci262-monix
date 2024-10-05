#include "CoreCommand.h"

#include <Util.h>

#include "commands/ExitCommand.h"
#include "commands/HelpCommand.h"
#include "commands/LoadCommand.h"
#include "commands/ShutdownCommand.h"
#include "commands/UnameCommand.h"

int MxInitializeCoreCommands()
{
    Logger.LogTrace("Initializing commands");

    // help
    MX_RETURN_IF_FAIL(MxCommandRegister("help", "Displays a list of available commands.",
        std::make_shared<HelpCommand, CommandStartupInfo&&>));

    // load
    MX_RETURN_IF_FAIL(MxCommandRegister("load", "Loads an external module.",
        std::make_shared<LoadCommand, CommandStartupInfo&&>));

    // uname
    MX_RETURN_IF_FAIL(MxCommandRegister("uname", "Prints system information.",
        std::make_shared<UnameCommand, CommandStartupInfo&&>));

    // exit/logout
    //
    // There actually is a slight difference between the two,
    // but currently Monix does not track whether a shell is login or not.
    MX_RETURN_IF_FAIL(MxCommandRegister("exit", "Exits the current shell.",
        std::make_shared<ExitCommand, CommandStartupInfo&&>));
    MX_RETURN_IF_FAIL(MxCommandRegister("logout", "Exits the current shell.",
        std::make_shared<ExitCommand, CommandStartupInfo&&>));

    // shutdown
    MX_RETURN_IF_FAIL(MxCommandRegister("shutdown", "Shuts down the Monix environment.",
        std::make_shared<ShutdownCommand, CommandStartupInfo&&>));

    return 0;
}
