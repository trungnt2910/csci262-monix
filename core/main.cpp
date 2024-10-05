#include <iostream>

#include <signal.h>

#include <System.h>
#include <Util.h>

#include "AsyncPatchProtection.h"
#include "CoreCommand.h"
#include "CoreConsole.h"
#include "CoreLogger.h"
#include "CoreModule.h"
#include "CoreSystem.h"
#include "ProtectedRegion.h"
#include "Settings.h"
#include "Signal.h"

int main(int argc, const char* const* argv)
{
    MX_RETURN_IF_FAIL(MxInitializeSettings(argc, argv));

    MX_RETURN_IF_FAIL(MxInitializeLogger());
    MX_RETURN_IF_FAIL(MxInitializeProtectedRegion());
    MX_RETURN_IF_FAIL(MxInitializeAsyncPatchProtection());
    MX_RETURN_IF_FAIL(MxInitializeSystem());
    MX_RETURN_IF_FAIL(MxInitializeConsole());
    MX_RETURN_IF_FAIL(MxInitializeCoreCommands());
    MX_RETURN_IF_FAIL(MxInitializeModule());

    MX_RETURN_IF_FAIL(MxAsyncPatchProtectionStart());

    std::string kernelName;
    std::string kernelRelease;
    std::string kernelVersion;
    MX_RETURN_IF_FAIL(MxSystemGetKernelName(kernelName));
    MX_RETURN_IF_FAIL(MxSystemGetKernelRelease(kernelRelease));
    MX_RETURN_IF_FAIL(MxSystemGetKernelVersion(kernelVersion));

    Console.GetOutput()
        << kernelName << " version " << kernelRelease
            << " (compiled " << kernelVersion << ")" << std::endl
        << "Copyright <C> 2024 Trung Nguyen (ttn903) & Group 1" << std::endl
        << "Check out lxmonika & Windows Subsystem for Monix: "
            << "https://go.trungnt2910.com/monika" << std::endl
        << std::endl
        << "Type \"help\" for a list of available commands." << std::endl
        << std::endl;

    MX_RETURN_IF_FAIL(MxSignalHandlerAdd(SIGINT, [](int)
    {
        Console.Interrupt();
    }));

    return Console.Run();
}
