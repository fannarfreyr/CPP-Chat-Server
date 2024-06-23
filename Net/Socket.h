#pragma once
#include "socketHandle.h"
#include "NResult.h"
#include "IPVersion.h"
#include "SocketOption.h"
#include "IPEndpoint.h"
#include "constants.h"
#include "packet.h"

namespace Net
{
    class Socket
    {
        public: 
            Socket( IPVersion ipversion = IPVersion::IPv4,
                    SocketHandle handle = INVALID_SOCKET);
            NResult Create();
            NResult Close();
            NResult Bind(IPEndpoint endpoint);
            NResult Listen(IPEndpoint endpoint, int backlog = 5);
            NResult Accept(Socket & outSocket, IPEndpoint * endpoint = nullptr);
            NResult Connect(IPEndpoint endpoint);
            NResult Send(const void * data, int numOfBytes, int &bytesSent);
            NResult Receive(void *destination, int numOfBytes, int &bytesRecv);
            NResult SendAll(const void *data, int numOfBytes);
            NResult ReceiveAll(void *destination, int numOfBytes);
            NResult Send(Packet & packet);
            NResult Receive(Packet & packet);
            SocketHandle GetHandle();
            IPVersion GetIpVersion();
            NResult SetBlocking(bool isBlocking);
        private:
            NResult SetSocketOption(SocketOption option, BOOL value);
            IPVersion ipversion = IPVersion::IPv4;
            SocketHandle handle = INVALID_SOCKET;
    };
}