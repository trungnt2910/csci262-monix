#include "Logger.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

bool Logger::_Log(LogLevel level, const char* file, int line, const char* function)
{
    if (level < LOGGER_MINIMUM_LEVEL)
    {
        return false;
    }

    // TODO: File name check.

    _Lock();

    const char* levelStr = "???";

    switch (level)
    {
        case LogLevel::Trace:
            levelStr = "TRC";
        break;
        case LogLevel::Info:
            levelStr = "INF";
        break;
        case LogLevel::Warning:
            levelStr = "WRN";
        break;
        case LogLevel::Error:
            levelStr = "ERR";
        break;
    }

    _GetLogFile() << levelStr << " ";

    // Extract name from file path.
    std::string fileName = std::filesystem::path(file).filename();

    // Print the file name and line, limited to the column size.
    std::string fileNameAndLine = fileName + ':' + std::to_string(line);
    if (fileNameAndLine.size() > LOGGER_COLUMN_WIDTH)
    {
        fileNameAndLine.resize(LOGGER_COLUMN_WIDTH);
    }
    _GetLogFile() << std::setw(LOGGER_COLUMN_WIDTH) << std::left << fileNameAndLine << " ";

    // Print the function name, limited to the column size.
    std::string functionString = function;
    if (functionString.size() > LOGGER_COLUMN_WIDTH)
    {
        functionString.resize(LOGGER_COLUMN_WIDTH);
    }
    _GetLogFile() << std::setw(LOGGER_COLUMN_WIDTH) << std::left << functionString << " ";

    return true;
}

class LoggerImpl: public Logger
{
private:
    std::mutex m_mutex;
    std::ostream* m_logFile = &std::cerr;

private:
    virtual void _Lock() override
    {
        m_mutex.lock();
    }

    virtual void _Unlock() override
    {
        m_logFile->flush();
        m_mutex.unlock();
    }

    virtual std::ostream& _GetLogFile()
    {
        return *m_logFile;
    }

public:
    void _SetLogFile(std::ostream& logFile)
    {
        m_logFile = &logFile;
    }
};

static std::ofstream s_LogFile;
static LoggerImpl s_Implementation;
class Logger& Logger = s_Implementation;

int MxInitializeLogger()
{
    Logger.LogTrace("Initializing logger");

    s_LogFile.open("monix.log");
    if (!s_LogFile.is_open())
    {
        errno = ENOENT;
        return -1;
    }

    s_Implementation._SetLogFile(s_LogFile);

    return 0;
}

