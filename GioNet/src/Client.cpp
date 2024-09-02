#include "Client.h"

#include <assert.h>
#include "Socket.h"

bool GioNet::Client::Connect(std::shared_ptr<Socket> connectionSocket)
{
    assert(connectionSocket && connectionSocket->IsValid());
    return connectionSocket->Connect();
}
