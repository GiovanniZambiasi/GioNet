#include "Server.h"
#include <assert.h>
#include <string>

#include "Socket.h"

GioNet::Server::Server(const std::shared_ptr<Socket>& listenSocket)
    : listenSocket(listenSocket)
{
}

void GioNet::Server::Listen()
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Listen();
    printf("Server listening for connections...\n");
    SOCKET client = listenSocket->Accept();
    printf("Successfully connected to client %llu\n", client);
}
