#include <iostream>

#include <Command.h>
#include <Console.h>
#include <Module.h>
#include <Util.h>

class HelloCommand: public Command
{
public:
    HelloCommand(CommandStartupInfo info)
        : Command(std::move(info))
    {
        // No-op.
    }

    virtual int Run() override
    {
        GetStartupInfo().console.GetOutput()
            << "Hello World!" << std::endl;

        return 0;
    }
};

static int HeInitializeModule()
{
    MX_RETURN_IF_FAIL(MxCommandRegister("hello", "Prints a \"Hello World!\" message.",
        std::make_shared<HelloCommand, CommandStartupInfo>));
    return 0;
}

extern "C" const constinit ModuleInfo MX_MODULE_INFO_SYMBOL
{
    .name = "hello",
    .publisher = "ttn903",
    .init = HeInitializeModule
};
