#include <iostream>
#include "myServer.h"

int main(int argc, char* argv[])
{   
    char* ip_string;
    unsigned short port;
    if (argc > 2)
    {
        ip_string = argv[1];
        port = atoi(argv[2]);
    }
    else
    {
        std::cout << "Invalid number of arguments " << std::endl;
        exit(-1);
    }
    MyServer server;
    if (Network::Initialize())
    {
        std::cout << "Winsock API successfully intialized" << std::endl;


    if (server.Initialize(IPEndpoint(ip_string, port)))
    {
        while ((true))
        {
            server.Frame();
        }
        
    }
    }
    Network::Shutdown();
    system("pause");
    return 0;
}