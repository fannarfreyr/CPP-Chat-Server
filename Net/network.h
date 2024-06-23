#pragma once
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

namespace Net 
{
    class Network
    {
        public:
            static bool Initialize();
            static void Shutdown();
    };
}