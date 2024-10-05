#ifndef ASYNC_PATCH_PROTECTION_H_INCLUDED
#define ASYNC_PATCH_PROTECTION_H_INCLUDED

// Initializes the async patch protection subsystem.
int MxInitializeAsyncPatchProtection();

// Calls when all core initialization is done to activate patch protection.
int MxAsyncPatchProtectionStart();

// Triggers the asynchronous patch protection callback
// without blocking the current thread.
int MxAsyncPatchProtectionTrigger();

#endif // ASYNC_PATCH_PROTECTION_H_INCLUDED
