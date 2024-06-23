#pragma once
#include "Socket.h"
#include "ManagePacket.h"

namespace Net
{
    class TCPConnection
    {
    public:
        TCPConnection(Socket socket, IPEndpoint endpoint);
        TCPConnection() :socket(Socket()){};
        void Close();
        std::string ToString();
        Socket socket;
        ManagePacket pm_incoming;
        ManagePacket pm_outgoing;
        char buffer[Net::maxPacketSize];
    private:
        IPEndpoint endpoint;
        std::string stringRepr = "";
    };
}