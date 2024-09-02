#include "Server.h"

#include <assert.h>

#include "Socket.h"

void GioNet::Server::BindSocket(std::shared_ptr<Socket> socket)
{
    assert(socket->IsValid());
    listenSocket = socket;
    listenSocket->Bind();
}

void GioNet::Server::Listen()
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Listen();
    printf("Server listening for connections...\n");
    SOCKET client = listenSocket->Accept();
    printf("Successfully connected to client %llu\n", client);
}
