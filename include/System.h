#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <string>

int MxSystemGetCurrentUser(std::string& user);
int MxSystemGetHostName(std::string& host);
int MxSystemGetKernelName(std::string& name);
int MxSystemGetKernelRelease(std::string& release);
int MxSystemGetKernelVersion(std::string& version);
int MxSystemGetMachine(std::string& machine);
int MxSystemGetOperatingSystem(std::string& os);

#endif // SYSTEM_H_INCLUDED
