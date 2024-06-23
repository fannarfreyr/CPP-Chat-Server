#include "myClient.h"
#include <iostream>
#include <pthread.h>

void *input(void *client)
{
    myClient *c = static_cast<myClient *>(client);
    c->input();
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
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
    if (Network::Initialize())
    {
        myClient client;
        pthread_t threads[1];
        if (client.Connect(IPEndpoint(ip_string, port)))
        {
            while (client.IsConnected())
            {
                client.Frame();
                pthread_create(&threads[0], NULL, input, &client);
            }
        }
    }
    Network::Shutdown();
    system("pause");
    return 0;
}