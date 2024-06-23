#pragma once
#include <queue>
#include <memory>
#include "packet.h"

namespace Net
{
    enum ManagePacketTask
    {
        ProcessSize,
        ProcessContents
    };
    class ManagePacket
    {
    public:
    void Clear();
    bool hasPendingPackets();
    void Append(std::shared_ptr<Packet> p);
    std::shared_ptr<Packet> Retrieve();
    void Pop();
    void sizeOfQueue();

    uint16_t currPacketSize = 0;
    int currPacketExtractOffset = 0;
    ManagePacketTask currTask = ManagePacketTask::ProcessSize;
    private:
        std::queue<std::shared_ptr<Packet>> packets;
    };
}