#pragma once
#define WIN32_LEAN_AND_MEAN
#include "IPVersion.h"
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <stdint.h>

namespace Net
{
    class IPEndpoint
    {
    public:
        IPEndpoint(){};
        IPEndpoint(const char * ip, unsigned short port);
        IPEndpoint(sockaddr * addr);
        IPVersion GetIPVersion();
        std::vector<uint8_t> GetIPBytes();
        std::string GetHostName();
        std::string GetIPString();
        unsigned short GetPort();
        sockaddr_in GetSockaddrV4();
        sockaddr_in6 GetSockaddrV6();
        void Print();
    private:
        IPVersion ipversion = IPVersion::Unknown;
        std::string hostname = "";
        std::string ip_string = "";
        std::vector<uint8_t> ip_bytes;
        unsigned short port = 0;
    };
}