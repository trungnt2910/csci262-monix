#include "Settings.h"

#include <cstring>
#include <iostream>

#include <Logger.h>

static bool s_OptionUnguarded = false;

const bool& g_OptionUnguarded = s_OptionUnguarded;

int MxInitializeSettings(int argc, const char* const* argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-u") == 0)
        {
            s_OptionUnguarded = true;
        }
        else
        {
            Logger.LogWarning("Unrecognized option: ", argv[i]);
        }
    }

    return 0;
}
