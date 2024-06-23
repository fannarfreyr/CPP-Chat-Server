#pragma once
#include "path to /Net/includeMe.h"

class myClient : public Client
{
    bool processPacket(std::shared_ptr<Packet> packet) override;
    void onConnect() override;
};