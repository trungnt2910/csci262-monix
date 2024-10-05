#include "ProtectedRegion.h"

#include <cstring>
#include <memory>
#include <span>

#include <sys/mman.h>

#include <Logger.h>

#include "CoreUtils.h"

static void* s_ProtectedPages = nullptr;
static int* s_ProtectedPagesMagic = nullptr;
static std::span<ProtectedRegionInfo> s_ProtectedInfo;
static size_t s_ProtectedInfoCount = 0;
static char* s_ProtectedPagesFreeStart = nullptr;
static size_t s_ProtectedPagesSpaceLeft = 0;

MX_HIDDEN_FROM_MODULES
void* const& g_ProtectedPages = s_ProtectedPages;
MX_HIDDEN_FROM_MODULES
const std::span<ProtectedRegionInfo>& g_ProtectedInfo = s_ProtectedInfo;
MX_HIDDEN_FROM_MODULES
const size_t& g_ProtectedInfoCount = s_ProtectedInfoCount;

MX_HIDDEN_FROM_MODULES
int MxInitializeProtectedRegion()
{
    Logger.LogTrace("Initializing protected regions");

    s_ProtectedPages = mmap(nullptr, MX_PROTECTED_REGION_SIZE,
        PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    if (s_ProtectedPages == MAP_FAILED)
    {
        Logger.LogError("Failed to map protected region.");
        return -1;
    }

    s_ProtectedPagesFreeStart = (char*)s_ProtectedPages;
    s_ProtectedPagesSpaceLeft = MX_PROTECTED_REGION_SIZE;

    // Magic number.
    s_ProtectedPagesMagic = (int*)s_ProtectedPages;
    s_ProtectedPagesFreeStart += sizeof(int);
    s_ProtectedPagesSpaceLeft -= sizeof(int);

    // Protected regions index.
    ProtectedRegionInfo* infoPtr;
    infoPtr = (ProtectedRegionInfo*)std::align(
        alignof(ProtectedRegionInfo),
        sizeof(ProtectedRegionInfo) * MX_PROTECTED_REGION_MAX,
        (void*&)s_ProtectedPagesFreeStart,
        s_ProtectedPagesSpaceLeft
    );
    if (infoPtr == nullptr)
    {
        Logger.LogError("Failed to allocate space for pages info.");
        return -1;
    }

    s_ProtectedInfo = std::span<ProtectedRegionInfo>(infoPtr, MX_PROTECTED_REGION_MAX);
    s_ProtectedPagesFreeStart += s_ProtectedInfo.size_bytes();
    s_ProtectedPagesSpaceLeft -= s_ProtectedInfo.size_bytes();

    // Initialize some fields.
    *s_ProtectedPagesMagic = MX_PROTECTED_REGION_MAGIC;
    s_ProtectedInfoCount = 0;

    return 0;
}

MX_HIDDEN_FROM_MODULES
int MxProtectedRegionAdd(const std::string& name, const std::span<const char>& data)
{
    // name size, one null character, then data size.
    size_t spaceNeeded = name.size() + 1 + data.size();

    if (spaceNeeded > s_ProtectedPagesSpaceLeft)
    {
        Logger.LogError("Exhausted protected pages.");
        errno = ENOMEM;
        return -1;
    }

    if (s_ProtectedInfoCount >= s_ProtectedInfo.size())
    {
        Logger.LogError("Exhausted region info table.");
        errno = ENOMEM;
        return -1;
    }

    // Allocate a block of memory.
    char* chosenRegion = s_ProtectedPagesFreeStart;
    s_ProtectedPagesFreeStart += spaceNeeded;
    s_ProtectedPagesSpaceLeft -= spaceNeeded;

    // Allocate an index and fill in the table.
    int index = (int)s_ProtectedInfoCount;
    ++s_ProtectedInfoCount;

    s_ProtectedInfo[index].name = chosenRegion;
    s_ProtectedInfo[index].data = chosenRegion + name.size() + 1;
    s_ProtectedInfo[index].size = data.size();

    // Copy in the data.
    memcpy((void*)s_ProtectedInfo[index].name, name.c_str(), name.size() + 1);
    memcpy((void*)s_ProtectedInfo[index].data, data.data(), data.size());

    return index;
}

MX_HIDDEN_FROM_MODULES
int MxProtectedRegionGet(int index, std::span<const char>& data)
{
    if (index < 0 || (size_t)index >= s_ProtectedInfoCount)
    {
        errno = EINVAL;
        return -1;
    }

    Logger.LogTrace("Retrieving protected region ", index,
        " with data of ", s_ProtectedInfo[index].size,
        " bytes at ", (void*)s_ProtectedInfo[index].data);

    data = std::span<const char>(s_ProtectedInfo[index].data, s_ProtectedInfo[index].size);
    return 0;
}
