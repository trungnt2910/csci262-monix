#ifndef PROTECTED_REGION_H_INCLUDED
#define PROTECTED_REGION_H_INCLUDED

#include <string>
#include <span>

struct ProtectedRegionInfo
{
    const char* name;
    const char* data;
    size_t size;
};

#define MX_PROTECTED_REGION_MAX 128     // 128 max regions
#define MX_PROTECTED_REGION_SIZE 16384  // 4 standard pages (16384 bytes) for these regions
#define MX_PROTECTED_REGION_MAGIC (('M' << 24) | ('o' << 16) | ('n' << 8) | 'i')    // 'Moni'

int MxInitializeProtectedRegion();

// Adds a new protected region, filled with the specified data.
// Returns -1 on failure, or the new region ID on success.
int MxProtectedRegionAdd(const std::string& name, const std::span<const char>& data);

// Gets the protected region with the specified ID.
int MxProtectedRegionGet(int index, std::span<const char>& data);

// The pointer to the start of the protected pages.
extern void* const& g_ProtectedPages;
// The list of protected region headers.
extern const std::span<ProtectedRegionInfo>& g_ProtectedInfo;
// The number of active protected regions.
extern const size_t& g_ProtectedInfoCount;

#endif // PROTECTED_REGION_H_INCLUDED
