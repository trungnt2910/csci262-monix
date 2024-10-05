#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED

#include <string_view>

struct ModuleInfo
{
    std::string_view name;
    std::string_view publisher;
    int (*init)();
};

#define MX_MODULE_INFO_SYMBOL g_ModuleInfo
#define MX_MODULE_INFO_SYMBOL_NAME "g_ModuleInfo"

int MxModuleLoad(const std::string& name, const std::string& password);

#endif // MODULE_H_INCLUDED
