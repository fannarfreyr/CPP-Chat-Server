#include "TCPConnection.h"

namespace Net
{
    TCPConnection::TCPConnection(Socket socket, IPEndpoint endpoint)
        :socket(socket),endpoint(endpoint)
    {
        stringRepr = "[" + endpoint.GetIPString();
        stringRepr += ":" + std::to_string(endpoint.GetPort()) + "]";
    }

    void TCPConnection::Close()
    {
        socket.Close();
    }

    std::string TCPConnection::ToString()
    {
        return stringRepr;
    }
}
