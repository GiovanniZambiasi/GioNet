#include "Client.h"

#include <assert.h>
#include "Socket.h"

GioNet::Client::Client(const std::shared_ptr<Socket>& socket)
    : socket(socket)
{
}

void GioNet::Client::SayHello()
{
    auto buffer = "Hello, world!";
    socket->Send(buffer, strlen(buffer) + 1);
}
