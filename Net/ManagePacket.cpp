#include "ManagePacket.h"

namespace Net
{

void Net::ManagePacket::Clear()
{
    packets = std::queue<std::shared_ptr<Packet>>{};
}
bool ManagePacket::hasPendingPackets()
{
    return (!packets.empty());
}
void ManagePacket::Append(std::shared_ptr<Packet> p)
{
    packets.push(std::move(p));
}
std::shared_ptr<Packet> ManagePacket::Retrieve()
{
    std::shared_ptr<Packet> p = packets.front();
    return p;
}
void ManagePacket::Pop()
{
    packets.pop();
}

void ManagePacket::sizeOfQueue()
{
    std::cout << packets.size() << std::endl;
}
}