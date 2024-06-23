#include "myClient.h"
#include <iostream>
#include <pthread.h>

bool myClient::processPacket(std::shared_ptr<Packet> packet)
{
    switch (packet->GetPacketType())
    {
    case PacketType::PT_ChatMsg:
    {
        std::string chatmsg;
        *packet >> chatmsg;
        std::cout << "Chat Message: " << chatmsg << std::endl;
        break;
    }
    case PacketType::PT_IntArray:
    {
        // for receiving public keys
        break;
    }
    default:
        return false;
    }
    return true;
}

void myClient::onConnect()
{
    std::cout << "Successfully connected to the server" << std::endl;

    //TODO: send keys here probably through an array of integers, big int?

    //std::shared_ptr<Packet> connectPacket = std::make_shared<Packet>(PacketType::PT_IntArray);
    //*connectPacket << ;
    //connection.pm_outgoing.Append(connectPacket);
}
