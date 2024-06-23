#include "Server.h"
#include <iostream>

namespace Net
{


bool Server::Initialize(IPEndpoint ip)
{    
        master_fd.clear();
        connections.clear();

        listeningSocket = Socket(ip.GetIPVersion());
        if (listeningSocket.Create() == NResult::N_Success)
        {
            std::cout << "Socket Successfully created." << std::endl;
            // 127.0.0.1 if only this machine
            // 192.168.0.2 if only on my router
            // 0.0.0.0 if any machine can connect (needs ports to be open)
            if (listeningSocket.Listen(ip) == NResult::N_Success)
            {
                WSAPOLLFD listeningSocketFD = {};
                listeningSocketFD.fd = listeningSocket.GetHandle();
                listeningSocketFD.events = POLLRDNORM;
                listeningSocketFD.revents = POLLRDNORM;

                master_fd.push_back(listeningSocketFD);

                std::cout << "Socket successfully listening." << std::endl;
                return true;   
            }
            else
            {
                std::cerr << "Failed to listen" << std::endl;
            }
            listeningSocket.Close();
        }
        else
        {
            std::cerr << "Socket failed to create." << std::endl; 
        }
    return false;
}

void Server::Frame()
{

    for (int i =0; i < connections.size(); i++)
    {
        if (connections[i].pm_outgoing.hasPendingPackets())
        {
            master_fd[i+1].events = POLLRDNORM | POLLWRNORM;
        }
    }
    use_fd = master_fd;

    if (WSAPoll(use_fd.data(), use_fd.size(), 1) > 0)
    {
        WSAPOLLFD & listeningSocketFD = use_fd[0];
        if (listeningSocketFD.revents & POLLRDNORM)
        {
            Socket newConnSocket;
            IPEndpoint newConnEndpoint;
            if (listeningSocket.Accept(newConnSocket, &newConnEndpoint) == NResult::N_Success)
            {
                connections.emplace_back(TCPConnection(newConnSocket,newConnEndpoint));
                TCPConnection & acceptedConn = connections[connections.size()-1];
                WSAPOLLFD newConnFD = {};
                newConnFD.fd = newConnSocket.GetHandle();
                newConnFD.events = POLLRDNORM;
                newConnFD.revents = 0;
                master_fd.push_back(newConnFD);
                onConnect(acceptedConn);
            }
            else
            {
                std::cerr << "Failed to accept new connection" << std::endl;
            }
        }
        for (int i=use_fd.size()-1; i >= 1; i--)
        {
            int connectionIndex = i - 1;
            TCPConnection & conn = connections[connectionIndex];
            if (use_fd[i].revents & POLLERR)
            {
                CloseConnection(connectionIndex, "POLLERR");
                continue;
            }
            if (use_fd[i].revents & POLLHUP)
            {
                CloseConnection(connectionIndex, "POLLHUP");
                continue;
            }
            if (use_fd[i].revents & POLLNVAL)
            {
                CloseConnection(connectionIndex, "POLLNVAL");
                continue;
            }
            if (use_fd[i].revents & POLLRDNORM)
            {
                int bytesRecv = 0;

                if (conn.pm_incoming.currTask == ManagePacketTask::ProcessSize)
                {
                    bytesRecv = recv(use_fd[i].fd, (char*)&conn.pm_incoming.currPacketSize + conn.pm_incoming.currPacketExtractOffset, sizeof(uint16_t) - conn.pm_incoming.currPacketExtractOffset, 0);
                }
                else // contents
                {
                    bytesRecv = recv(use_fd[i].fd, (char*)&conn.buffer + conn.pm_incoming.currPacketExtractOffset, conn.pm_incoming.currPacketSize - conn.pm_incoming.currPacketExtractOffset, 0);
                }

                if (bytesRecv == 0)
                {
                    CloseConnection(connectionIndex, "Recv==0");
                    continue;
                }
                if (bytesRecv == SOCKET_ERROR)
                {
                    int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK)
                    {
                        CloseConnection(connectionIndex, "Recv<0");
                        continue;
                    }
                }

                if (bytesRecv > 0)
                {
                    conn.pm_incoming.currPacketExtractOffset += bytesRecv;
                    if (conn.pm_incoming.currTask == ManagePacketTask::ProcessSize)
                    {
                        if (conn.pm_incoming.currPacketExtractOffset == sizeof(uint16_t))
                        {
                            conn.pm_incoming.currPacketSize = ntohs(conn.pm_incoming.currPacketSize);
                            if (conn.pm_incoming.currPacketSize > Net::maxPacketSize)
                            {
                                CloseConnection(connectionIndex, "Packet size too large");
                                continue;;
                            }
                            conn.pm_incoming.currPacketExtractOffset = 0;
                            conn.pm_incoming.currTask = ManagePacketTask::ProcessContents;
                        }
                    }
                    else // contents
                    {
                        if (conn.pm_incoming.currPacketExtractOffset == conn.pm_incoming.currPacketSize)
                        {
                            std::shared_ptr<Packet> packet = std::make_shared<Packet>();
                            packet->buffer.resize(conn.pm_incoming.currPacketSize);
                            memcpy(&packet->buffer[0], conn.buffer, conn.pm_incoming.currPacketSize);

                            conn.pm_incoming.Append(packet);
                            conn.pm_incoming.currPacketSize = 0;
                            conn.pm_incoming.currPacketExtractOffset = 0;
                            conn.pm_incoming.currTask = ManagePacketTask::ProcessSize;
                        }
                    }
                }
            }
            if (use_fd[i].revents & POLLWRNORM)
            {
                ManagePacket pm = conn.pm_outgoing;
                while (pm.hasPendingPackets())
                {
                    if (pm.currTask == ManagePacketTask::ProcessSize)
                    {
                        pm.currPacketSize = pm.Retrieve()->buffer.size();
                        uint16_t bEPacketSize = htons(pm.currPacketSize);
                        int bytesSent = send(use_fd[i].fd, (char*)(&bEPacketSize)+pm.currPacketExtractOffset, sizeof(uint16_t) - pm.currPacketExtractOffset, 0);

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
                        int bytesSent = send(use_fd[i].fd, (char*)(bufferPtr)+pm.currPacketExtractOffset, pm.currPacketSize - pm.currPacketExtractOffset, 0);
                        if (bytesSent > 0)
                        {
                            pm.currPacketExtractOffset += bytesSent;
                        }
                        if (pm.currPacketExtractOffset == pm.currPacketSize)
                        {
                            pm.currPacketExtractOffset = 0;
                            pm.currTask = ManagePacketTask::ProcessSize;
                            pm.Pop();
                            conn.pm_outgoing.Pop();
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                if (!pm.hasPendingPackets())
                {
                    master_fd[i].events = POLLRDNORM;
                }
            }
        }
    }
        for (int i = connections.size() - 1; i>= 0; i--)
        {
            
            while (connections[i].pm_incoming.hasPendingPackets())
            {

                std::shared_ptr<Packet> frontPacket = connections[i].pm_incoming.Retrieve();
                if (!ProcessPacket(connections[i],frontPacket))
                {
                    CloseConnection(i, "Failed to process incoming packet");
                    break;
                }
                connections[i].pm_incoming.Pop();
            }
        }
}

void Server::CloseConnection(int connectionIndex, std::string reason)
{
    TCPConnection & conn = connections[connectionIndex];
    onDisconnect(conn, reason);
    master_fd.erase(master_fd.begin() + (connectionIndex + 1));
    use_fd.erase(use_fd.begin() + (connectionIndex + 1));
    conn.Close();
    connections.erase(connections.begin() + connectionIndex);   
}

void Server::onConnect(TCPConnection &newConn)
{
    std::cout << newConn.ToString() << " - New connection accepted" << std::endl;
}

void Server::onDisconnect(TCPConnection &lostConn, std::string reason)
{
    std::cout << "[" << reason << "] Connection lost: " << lostConn.ToString() << std::endl;
}

bool Server::ProcessPacket(TCPConnection & sender,std::shared_ptr<Packet> packet)
{
    std::cout << "Packet received with size: " << packet->buffer.size() << std::endl;
    return true;
}
}