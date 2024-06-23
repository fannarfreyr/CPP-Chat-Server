#include "Socket.h"
#include "packet.h"
#include <assert.h>
#include <iostream>


namespace Net
{
    Socket::Socket(IPVersion ipversion, SocketHandle handle):ipversion(ipversion), handle(handle)
    {
        assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
    };
    NResult Socket::Create()
    {
        assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);

        if (handle != INVALID_SOCKET)
        {
            return NResult::N_Error;
        }

        handle = socket((ipversion == IPVersion::IPv4) ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        if (handle == INVALID_SOCKET)
        {
            return NResult::N_Error;
        }

        if (SetBlocking(false) != NResult::N_Success)
        {
            return NResult::N_Error;
        }

        if (SetSocketOption(SocketOption::TCP_NoDelay, TRUE) != NResult::N_Success)
        {
            return NResult::N_Error;
        }

        return NResult::N_Success;
    };

    NResult Socket::Close()
    {
        if (handle == INVALID_SOCKET)
        {
            return NResult::N_Error;
        }

        int iResult = closesocket(handle);
        
        if (iResult != 0) // if error while closing
        {
            return NResult::N_Error;
        }

        handle = INVALID_SOCKET;
        return NResult::N_Success;
    }
    NResult Socket::Bind(IPEndpoint endpoint)
    {
        assert(ipversion == endpoint.GetIPVersion());
        if (ipversion == IPVersion::IPv4)
        {
            sockaddr_in addr = endpoint.GetSockaddrV4();
            int result = bind(handle, (sockaddr*)&addr, sizeof(sockaddr_in));
            if (result != 0)
            {
                return NResult::N_Error;
            }
            return NResult::N_Success;
        }
        else // ipv6
        {
            sockaddr_in6 addr = endpoint.GetSockaddrV6();
            int result = bind(handle, (sockaddr*)&addr, sizeof(sockaddr_in6));
            if (result != 0)
            {
                return NResult::N_Error;
            }
            return NResult::N_Success;
        }
    }
    NResult Socket::Listen(IPEndpoint endpoint, int backlog)
    {
        if (ipversion == IPVersion::IPv6)
        {
            if (SetSocketOption(SocketOption::IPV6_Only, FALSE) != NResult::N_Success)
            {
                std::cout << "Failed" << std::endl;
                return NResult::N_Error;
            }
        }
        if (Bind(endpoint) != NResult::N_Success)
        {
            return NResult::N_Error;
        }

        int result = listen(handle, backlog);

        if (result != 0)
        {
            return NResult::N_Error;
        }
        return NResult::N_Success;
    }

    NResult Socket::Accept(Socket &outSocket, IPEndpoint *endpoint)
    {
        assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);

        if (ipversion == IPVersion::IPv4)
        {
            sockaddr_in addr {};
            int len = sizeof(sockaddr_in);
            SocketHandle acceptedConnHandle = accept(handle, (sockaddr*)&addr, &len);
            if (acceptedConnHandle == INVALID_SOCKET)
            {
                return NResult::N_Error;
            }
            if (endpoint != nullptr)
            {
                *endpoint = IPEndpoint((sockaddr*)&addr);
            }
            
            outSocket = Socket(IPVersion::IPv4, acceptedConnHandle);
        }
        else // ipv6
        {
            sockaddr_in6 addr {};
            int len = sizeof(sockaddr_in6);
            SocketHandle acceptedConnHandle = accept(handle, (sockaddr*)&addr, &len);
            if (acceptedConnHandle == INVALID_SOCKET)
            {
                return NResult::N_Error;
            }
            if (endpoint != nullptr)
            {
                *endpoint = IPEndpoint((sockaddr*)&addr);
            }
            
            outSocket = Socket(IPVersion::IPv6, acceptedConnHandle);
        }
        
        return NResult::N_Success;
    }
    NResult Socket::Connect(IPEndpoint endpoint)
    {
        assert(ipversion == endpoint.GetIPVersion());
        int result = 0;
        if (ipversion == IPVersion::IPv4)
        {
            sockaddr_in addr = endpoint.GetSockaddrV4();
            result = connect(handle, (sockaddr*)&addr, sizeof(sockaddr_in));
        }
        else // ipv6
        {
            sockaddr_in6 addr = endpoint.GetSockaddrV6();
            result = connect(handle, (sockaddr*)&addr, sizeof(sockaddr_in6));
        }
        if (result != 0)
        {
            return NResult::N_Error;
        }
        
        return NResult::N_Success;
    }
    NResult Socket::Send(const void *data, int numOfBytes, int &bytesSent)
    {
        bytesSent = send(handle, (const char*)data, numOfBytes, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            return NResult::N_Error;
        }
        return NResult::N_Success;
    }
    NResult Socket::Receive(void *destination, int numOfBytes, int &bytesRecv)
    {
        bytesRecv = recv(handle, (char*)destination, numOfBytes, 0);
        if (bytesRecv == 0)
        {
            return NResult::N_Error;
        }
        if (bytesRecv == SOCKET_ERROR)
        {
            return NResult::N_Error;
        }
        return NResult::N_Success;
    }
    NResult Socket::SendAll(const void *data, int numOfBytes)
    {
        int totalBytesSent = 0;
        while (totalBytesSent < numOfBytes)
        {
            int bytesRemaining = numOfBytes - totalBytesSent;
            int bytesSent = 0;
            char * bufOffset = (char*)data + totalBytesSent;
            NResult result = Send(bufOffset, bytesRemaining, bytesSent);

            if (result != NResult::N_Success)
            {
                return NResult::N_Error;
            }
            totalBytesSent += bytesSent;
        }
        return NResult::N_Success;
    }
    NResult Socket::ReceiveAll(void *destination, int numOfBytes)
    {
        int totalBytesRecv = 0;
        while (totalBytesRecv < numOfBytes)
        {
            int bytesRemaining = numOfBytes - totalBytesRecv;
            int bytesRecv = 0;
            char * bufOffset = (char*)destination + totalBytesRecv;
            NResult result = Receive(bufOffset, bytesRemaining, bytesRecv);

            if (result != NResult::N_Success)
            {
                return NResult::N_Error;
            }
            totalBytesRecv += bytesRecv;
        }
        return NResult::N_Success;
    }
    NResult Socket::Send(Packet &packet)
    {
        uint16_t encodedPacketSize = htons(packet.buffer.size());
        NResult result = SendAll(&encodedPacketSize, sizeof(uint16_t));
        if(result != NResult::N_Success)
            return NResult::N_Error;
        
        result = SendAll(packet.buffer.data(), packet.buffer.size());
        if(result != NResult::N_Success)
            return NResult::N_Error;
        return NResult::N_Success;
    }
    NResult Socket::Receive(Packet &packet)
    {
        packet.Clear();

        uint16_t encodedSize = 0;
        NResult result = ReceiveAll(&encodedSize, sizeof(uint16_t));
        if(result != NResult::N_Success)
            return NResult::N_Error;

        uint32_t bufferSize = ntohs(encodedSize);

        if (bufferSize > Net::maxPacketSize)
            return NResult:: N_Error;

        packet.buffer.resize(bufferSize);
        result = ReceiveAll(&packet.buffer[0], bufferSize);
        if(result != NResult::N_Success)
            return NResult::N_Error;
        return NResult::N_Success;
    };

    NResult Socket::SetSocketOption(SocketOption option, BOOL value)
    {
        int iResult = 0;
        switch (option)
        {
        case SocketOption::TCP_NoDelay:
            iResult = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
            break;
        case SocketOption::IPV6_Only:
            iResult = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
            break;

        default:
            return NResult::N_Error;
            break;
        }

        if (iResult != 0){            
            return NResult::N_Error;
        }

        return NResult::N_Success;
    }


    IPVersion Socket::GetIpVersion()
    {
        return ipversion;
    }
    NResult Socket::SetBlocking(bool isBlocking)
    {
        unsigned long nonBlocking = 1;
        unsigned long blocking = 0;
        int result = ioctlsocket(handle, FIONBIO, isBlocking ?  &blocking : &nonBlocking);
        if (result == SOCKET_ERROR)
        {
            return NResult::N_Error;
        }
        return NResult::N_Success;
    };

    SocketHandle Socket::GetHandle()
    {
        return handle;
    };
}