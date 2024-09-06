#include "Server.h"
#include <assert.h>
#include <string>
#include "Socket.h"

std::string GioNet::Peer::ToString() const
{
    return connection ? connection->ToString() : address.ToString();
}

GioNet::Server::Server(const std::shared_ptr<Socket>& listenSocket)
    : listenSocket(listenSocket)
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Bind();
}

GioNet::Server::~Server()
{
    Stop();
}

void GioNet::Server::Start()
{
    assert(listenSocket && listenSocket->IsValid());
}

void GioNet::Server::Stop()
{
    listenSocket->Close();
}

void GioNet::Server::AddPeer(const Peer& peer)
{
    std::unique_lock _{peersMutex};
    peers.push_back(peer);
    OnPostPeerAdded(peer);
    printf("Successfully connected to peer %s\n", peer.ToString().c_str());
}

void GioNet::Server::RemovePeer(const Peer& peer)
{
    std::unique_lock _{peersMutex};

    auto peerEntry = std::find(peers.begin(), peers.end(), peer); 
    if(peerEntry != peers.end())
    {
        printf("Disconnecting peer %s\n", peer.ToString().c_str());
        OnPrePeerRemoved(peer);
        peers.erase(peerEntry);
    }
    else
    {
        printf("[ERROR]: Tried to remove peer not in connections list: '%s'\n", peer.ToString().c_str());
    }
}
