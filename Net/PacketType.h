#pragma once
#include <stdint.h>

namespace Net
{
    enum PacketType : uint16_t
    {
        PT_Invalid,
        PT_ChatMsg,
        PT_IntArray,
        PT_Test,
    };
}