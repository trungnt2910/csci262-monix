#ifndef SIGNAL_H_INCLUDED
#define SIGNAL_H_INCLUDED

#include <functional>

int MxInitializeSignal();
int MxSignalHandlerAdd(int sig, std::function<void(int)> handler);

#define MX_ALL_SIGNALS (-1)

#endif // SIGNAL_H_INCLUDED
