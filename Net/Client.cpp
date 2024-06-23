#include "Client.h"
#include <iostream>
#include "TCPConnection.h"


namespace Net
{

bool Client::Connect(IPEndpoint ip)
{
    isConnected = false;
    
    Socket socket = Socket(ip.GetIPVersion());

    if (socket.Create() == NResult::N_Success)
    {
        if (socket.SetBlocking(true) != NResult::N_Success)
        {
            return false;
        }
        std::cout << "Socket Successfully created." << std::endl;
        if (socket.Connect(ip) == NResult::N_Success)
        {
            if (socket.SetBlocking(false) == NResult::N_Success)
            {
                connection = TCPConnection(socket, ip);
                master_fd.fd = connection.socket.GetHandle();
                master_fd.events = POLLRDNORM;
                master_fd.revents = 0;
                isConnected = true;
                onConnect();
                return true;            
            }
        }
        else
        {
        }
        socket.Close();
    }
    else
    {
        std::cerr << "Socket failed to create." << std::endl; 
    }
    onConnectFail();
    return false;
}


bool Client::IsConnected()
{
    return isConnected;
}

bool Client::Frame()
{   
    if (connection.pm_outgoing.hasPendingPackets())
    {
        master_fd.events = POLLRDNORM | POLLWRNORM;
    }
    use_fd = master_fd;

    if (WSAPoll(&use_fd, 1, 1) > 0)
    {
            if (use_fd.revents & POLLERR)
            {
                CloseConnection("POLLERR");
                return false;
            }
            if (use_fd.revents & POLLHUP)
            {
                CloseConnection("POLLHUP");
                return false;
            }
            if (use_fd.revents & POLLNVAL)
            {
                CloseConnection("POLLNVAL");
                return false;
            }

            
            if (use_fd.revents & POLLRDNORM)
            {
                int bytesRecv = 0;

                if (connection.pm_incoming.currTask == ManagePacketTask::ProcessSize)
                {
                    bytesRecv = recv(use_fd.fd, (char*)&connection.pm_incoming.currPacketSize + connection.pm_incoming.currPacketExtractOffset, sizeof(uint16_t) - connection.pm_incoming.currPacketExtractOffset, 0);
                    
                }
                else // contents
                {
                    bytesRecv = recv(use_fd.fd, (char*)&connection.buffer + connection.pm_incoming.currPacketExtractOffset, connection.pm_incoming.currPacketSize - connection.pm_incoming.currPacketExtractOffset, 0);

                }

                if (bytesRecv == 0)
                {
                    CloseConnection("Recv==0");
                    return false;
                }
                if (bytesRecv == SOCKET_ERROR)
                {
                    int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK)
                    {
                        CloseConnection("Recv<0");
                        return false;
                    }
                }

                if (bytesRecv > 0)
                {
                    connection.pm_incoming.currPacketExtractOffset += bytesRecv;
                    if (connection.pm_incoming.currTask == ManagePacketTask::ProcessSize)
                    {
                        if (connection.pm_incoming.currPacketExtractOffset == sizeof(uint16_t))
                        {
                            connection.pm_incoming.currPacketSize = ntohs(connection.pm_incoming.currPacketSize);
                            if (connection.pm_incoming.currPacketSize > Net::maxPacketSize)
                            {
                                CloseConnection("Packet size too large");
                                return false;
                            }
                            connection.pm_incoming.currPacketExtractOffset = 0;
                            connection.pm_incoming.currTask = ManagePacketTask::ProcessContents;
                        }
                    }
                    else // contents
                    {
                        if (connection.pm_incoming.currPacketExtractOffset == connection.pm_incoming.currPacketSize)
                        {

                            std::shared_ptr<Packet> packet = std::make_shared<Packet>();
                            packet->buffer.resize(connection.pm_incoming.currPacketSize);
                            memcpy(&packet->buffer[0], connection.buffer, connection.pm_incoming.currPacketSize);

                            connection.pm_incoming.Append(packet);

                            connection.pm_incoming.currPacketSize = 0;
                            connection.pm_incoming.currPacketExtractOffset = 0;
                            connection.pm_incoming.currTask = ManagePacketTask::ProcessSize;
                        }
                    }
                }
            }
            if (use_fd.revents & POLLWRNORM)
            {
                ManagePacket pm = connection.pm_outgoing;
                while (pm.hasPendingPackets())
                {
                    if (pm.currTask == ManagePacketTask::ProcessSize)
                    {
                        pm.currPacketSize = pm.Retrieve()->buffer.size();
                        uint16_t bEPacketSize = htons(pm.currPacketSize);
                        int bytesSent = send(use_fd.fd, (char*)(&bEPacketSize)+pm.currPacketExtractOffset, sizeof(uint16_t) - pm.currPacketExtractOffset, 0);
                        if (bytesSent > 0)
                        {
                            pm.currPacketExtractOffset += bytesSent;
                        }
                        if (pm.currPacketExtractOffset == sizeof(uint16_t))
                        {
                            pm.currPacketExtractOffset = 0;
                            pm.currTask = ManagePacketTask::ProcessContents;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else // contents
                    {

                        char * bufferPtr = &pm.Retrieve()->buffer[0];
                        int bytesSent = send(use_fd.fd, (char*)(bufferPtr)+pm.currPacketExtractOffset, pm.currPacketSize - pm.currPacketExtractOffset, 0);
                        if (bytesSent > 0)
                        {
                            pm.currPacketExtractOffset += bytesSent;
                        }
                        if (pm.currPacketExtractOffset == pm.currPacketSize)
                        {
                            pm.currPacketExtractOffset = 0;
                            pm.currTask = ManagePacketTask::ProcessSize;
                            connection.pm_outgoing.Pop();
                            pm.Pop();
                        }
                        else
                        {
                            break;
                        }
                    }
                } 
                if (!connection.pm_outgoing.hasPendingPackets())
                {
                    master_fd.events = POLLRDNORM;
                }      
            }
        }
                        
        while (connection.pm_incoming.hasPendingPackets())
        {
            std::shared_ptr<Packet> frontPacket = connection.pm_incoming.Retrieve();
            if (!processPacket(frontPacket))
            {
                CloseConnection("Failed to process incoming packet");
                return false;
            }
            connection.pm_incoming.Pop();
        }
        return false;
}

bool Client::processPacket(std::shared_ptr<Packet> packet)
{
    std::cout << "Packet received with size: " << packet->buffer.size() << std::endl;
    return true;
}

void Client::onConnect()
{
    std::cout << "Successfully connected!" << std::endl;

}

void Client::onConnectFail()
{
    std::cout << "Failed to connect" << std::endl;

}

void Client::onDisconnect(std::string reason)
{
    std::cout << "Lost connection. Reason: " << reason << std::endl;

}

void Client::input()
{
    std::string user_input;
    std::getline(std::cin, user_input);

    if (user_input == std::string("!exit"))
    {
        CloseConnection("User decided to quit");
        return;
    }

    std::shared_ptr<Packet> messagePacket = std::make_shared<Packet>(PacketType::PT_ChatMsg);
    *messagePacket << user_input;
    connection.pm_outgoing.Append(messagePacket);
}

void Client::CloseConnection(std::string reason)
{
    onDisconnect(reason);
    master_fd.fd = 0;
    isConnected = false;
    connection.Close();
}
}