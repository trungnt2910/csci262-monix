#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

#include <ext/stdio_filebuf.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <Console.h>
#include <Logger.h>
#include <Module.h>

class TcpBackdoor: public Console
{
private:
    static constexpr int port = 6969;

    __gnu_cxx::stdio_filebuf<char> m_tcpFileBufIn;
    __gnu_cxx::stdio_filebuf<char> m_tcpFileBufOut;
    std::istream m_tcpFileIn;
    std::ostream m_tcpFileOut;
public:
    TcpBackdoor(int fd)
        : Console(m_tcpFileIn, m_tcpFileOut),
          m_tcpFileBufIn(dup(fd), std::ios_base::in),
          m_tcpFileBufOut(dup(fd), std::ios_base::out),
          m_tcpFileIn(&m_tcpFileBufIn),
          m_tcpFileOut(&m_tcpFileBufOut)
    {
        // No-op.
    }

    virtual std::istream& GetInput() override
    {
        return m_tcpFileIn;
    }

    virtual std::ostream& GetOutput() override
    {
        return m_tcpFileOut;
    }
};

static int TbInitializeModule()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        Logger.LogTrace("Failed to open socket: ", strerror(errno), '.');
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6969);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == -1)
    {
        Logger.LogTrace("Failed to bind: ", strerror(errno), '.');
        return -1;
    }

    if (listen(sock, 16) == -1)
    {
        Logger.LogTrace("Failed to listen: ", strerror(errno), '.');
        return -1;
    }

    while (true)
    {
        sockaddr_in sourceAddr;
        socklen_t sourceAddrLen = sizeof(sourceAddr);
        int fd = accept(sock, (sockaddr*)&sourceAddr, &sourceAddrLen);
        if (fd == -1)
        {
            Logger.LogTrace("Failed to accept: ", strerror(errno), '.');
            continue;
        }

        std::thread([=]()
        {
            TcpBackdoor backdoor(fd);
            try
            {
                backdoor.Run();
            }
            catch (...)
            {
                // Ignore.
            }

            shutdown(fd, SHUT_RDWR);
            close(fd);
        }).detach();
    }

    return 0;
}

extern "C" const constinit ModuleInfo MX_MODULE_INFO_SYMBOL
{
    .name = "tobira",
    .publisher = "Evil",
    .init = TbInitializeModule
};
