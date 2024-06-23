#include "network.h"
#include <iostream>

using namespace Net;

bool Network::Initialize(){
    WSAData wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0){
        std::cerr << "Failed to start up the winsock API." << std::endl;
        return false;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) !=2){
        std::cerr << "Could not find a usable version of the winsock API dll." << std::endl;
        return false;
    }
    return true;
}

void Network::Shutdown(){
    WSACleanup();
}