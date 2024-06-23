#include "packet.h"
#include "constants.h"

namespace Net
{
    Packet::Packet(PacketType packetType)
    {
        Clear();
        AssignPacketType(packetType);
    }

    PacketType Packet::GetPacketType()
    {
        PacketType * packetTypePtr = reinterpret_cast<PacketType*>(&buffer[0]);
        return static_cast<PacketType>(ntohs(*packetTypePtr));
    }

    void Packet::AssignPacketType(PacketType packetType)
    {
        PacketType * packetTypePtr = reinterpret_cast<PacketType*>(&buffer[0]);
        *packetTypePtr = static_cast<PacketType>(htons(packetType));
    }

void Packet::Clear()
{
    buffer.resize(sizeof(PacketType));
    AssignPacketType(PacketType::PT_Invalid);
    extractOffset = sizeof(PacketType);
}

void Packet::Append(const void *data, uint32_t size)
{
    if ((buffer.size() + size) > Net::maxPacketSize)
        throw PacketException("[Packet::Append(const void*, uint32_t)] - Packet size exceeded max packet size.");

    buffer.insert(buffer.end(), (char*)data, (char*)data + size);
}

Packet &Packet::operator<<(uint32_t data)
{
    data = htonl(data);
    Append(&data, sizeof(uint32_t));
    return *this;
}

Packet &Packet::operator>>(uint32_t &data)
{
    if ((extractOffset + sizeof(uint32_t)) > buffer.size())
        throw PacketException("[Packet::operator >>(uint32_t &)] - Extraction offset exceeded the buffer size.");
    data = *reinterpret_cast<uint32_t*>(&buffer[extractOffset]);
    data = ntohl(data);
    extractOffset += sizeof(uint32_t);
    return *this;
}
Packet &Packet::operator<<(const std::string &data)
{
    *this << (uint32_t)data.size();
    Append(data.data(), data.size());
    return *this;
}
Packet &Packet::operator>>(std::string &data)
{
    data.clear();
    uint32_t stringSize = 0;
    *this >> stringSize;

    if ((extractOffset + stringSize) > buffer.size())
        throw PacketException("[Packet::operator (std::string &)] - Extraction offset exceeded the buffer size.");

    data.resize(stringSize);
    data.assign(&buffer[extractOffset], stringSize);
    extractOffset += stringSize;
    return *this;
}
}
