#pragma once
#include "TCPConnection.h"

using namespace Net;

namespace Net
{

class Client
{
public:
    bool Connect(IPEndpoint ip);
    bool IsConnected();
    bool Frame();
    void input();
protected:
    virtual bool processPacket(std::shared_ptr<Packet> packet);
    virtual void onConnect();
    virtual void onConnectFail();
    virtual void onDisconnect(std::string reason);
    TCPConnection connection;
    void CloseConnection(std::string reason);
private:
    bool isConnected = false;
    WSAPOLLFD master_fd;
    WSAPOLLFD use_fd;
   
};
}

