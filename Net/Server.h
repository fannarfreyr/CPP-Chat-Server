#pragma once
#include "TCPConnection.h"

using namespace Net;

namespace Net
{

class Server
{
public:
    bool Initialize(IPEndpoint ip);
    void Frame();
protected:
    virtual bool ProcessPacket(TCPConnection & sender,std::shared_ptr<Packet> packet);
    virtual void onConnect(TCPConnection & newConn);
    virtual void onDisconnect(TCPConnection & lostConn, std::string reason);

    void CloseConnection(int connectionIndex, std::string reason);
    Socket listeningSocket;
    std::vector<TCPConnection> connections;
    std::vector<WSAPOLLFD> master_fd;
    std::vector<WSAPOLLFD> use_fd;
};
}