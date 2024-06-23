#pragma once

#include <path to /Net/includeMe.h>

class MyServer : public Server
{
    private:
        void onConnect(TCPConnection & newConn) override;
        void onDisconnect(TCPConnection & lostConn, std::string reason) override;
        bool ProcessPacket(TCPConnection & sender,std::shared_ptr<Packet> packet) override;
};