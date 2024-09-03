#include "Client.h"

#include <assert.h>
#include "Socket.h"

GioNet::Client::Client(const std::shared_ptr<Socket>& socket)
    : socket(socket)
{
}
