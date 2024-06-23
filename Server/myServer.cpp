#include "myServer.h"
#include <iostream>

void MyServer::onConnect(TCPConnection &newConn)
{
    std::cout << newConn.ToString() << " - New connection accepted" << std::endl;

    std::shared_ptr<Packet> welcomePacket = std::make_shared<Packet>(PacketType::PT_ChatMsg);
    *welcomePacket << std::string("Welcome!");
    newConn.pm_outgoing.Append(welcomePacket);

    std::shared_ptr<Packet> newUsrPacket = std::make_shared<Packet>(PacketType::PT_ChatMsg);
    *newUsrPacket << std::string("A new user connected");
    for (auto & connection : connections)
    {
        if (&connection == &newConn)
        {
            continue;
        }
        connection.pm_outgoing.Append(newUsrPacket);
    }
}

void MyServer::onDisconnect(TCPConnection &lostConn, std::string reason)
{
    std::cout << "[" << reason << "] Connection lost: " << lostConn.ToString() << std::endl;

    std::shared_ptr<Packet> connLostPacket = std::make_shared<Packet>(PacketType::PT_ChatMsg);
    *connLostPacket << std::string("A user disconnected");
    for (auto & connection : connections)
    {
        if (&connection == &lostConn)
        {
            continue;
        }
        connection.pm_outgoing.Append(connLostPacket);
    }

}

bool MyServer::ProcessPacket(TCPConnection &sender ,std::shared_ptr<Packet> packet)
{
    switch (packet->GetPacketType())
    {
    case PacketType::PT_ChatMsg:
    {
        std::string chatmsg;
        *packet >> chatmsg;
        std::cout << sender.ToString() << " - " << chatmsg << std::endl;
        
        break;
    }
    case PacketType::PT_IntArray:
    {
        // for public keys, RSA?
        break;
    }
    
    default:
        std::cout << "Unrecognized packet type: " << packet->GetPacketType() << std::endl;
        return false;
    }
    for (auto & connection : connections)
        {
            if (&connection == &sender)
            {
                continue;
            }
            connection.pm_outgoing.Append(packet);
        }
    return true;
}
