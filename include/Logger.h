#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <iosfwd>

enum class LogLevel
{
    Trace,
    Info,
    Warning,
    Error
};

#ifndef NDEBUG
#define LOGGER_MINIMUM_LEVEL (LogLevel::Trace)
#else
#define LOGGER_MINIMUM_LEVEL (LogLevel::Info)
#endif

#define LOGGER_COLUMN_WIDTH 32

class Logger
{
private:
    template <typename TFirst, typename... TRest>
    void _Print(TFirst first, TRest... args)
    {
        _GetLogFile() << first;
        _Print(args...);
    }

    void _Print() {}

private:
    virtual void _Lock() = 0;
    virtual void _Unlock() = 0;

    virtual std::ostream& _GetLogFile() = 0;
public:
    bool _Log(LogLevel level, const char* file, int line, const char* function);

    template <LogLevel level, typename... TRest>
    bool _Log(const char* file, int line, const char* function,
        [[maybe_unused]] TRest... args)
    {
        if constexpr (level < LOGGER_MINIMUM_LEVEL)
        {
            (void)file;
            (void)line;
            (void)function;
            return false;
        }
        else
        {
            if (!_Log(level, file, line, function))
            {
                return false;
            }

            _Print(args...);
            _Print("\n");

            _Unlock();
            return true;
        }
    }
};

extern class Logger& Logger;

#define LogTrace(...)           \
    _Log<LogLevel::Trace>(      \
        __FILE__,               \
        __LINE__,               \
        __func__,               \
        __VA_ARGS__             \
    )

#define LogInfo(...)            \
    _Log<LogLevel::Info>(       \
        __FILE__,               \
        __LINE__,               \
        __func__,               \
        __VA_ARGS__             \
    )

#define LogWarning(...)         \
    _Log<LogLevel::Warning>(    \
        __FILE__,               \
        __LINE__,               \
        __func__,               \
        __VA_ARGS__             \
    )

#define LogError(...)           \
    _Log<LogLevel::Error>(      \
        __FILE__,               \
        __LINE__,               \
        __func__,               \
        __VA_ARGS__             \
    )

#endif // LOGGER_H_INCLUDED
