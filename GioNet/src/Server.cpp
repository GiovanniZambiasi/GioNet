#include "Server.h"
#include <ranges>
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

void GioNet::Server::Broadcast(const Buffer& buffer)
{
    const auto& peerAddresses = std::views::keys(peers);

    for(const NetAddress& address : peerAddresses)
    {
        auto peer = peers.find(address);

        if(peer != peers.end())
        {
            Send(buffer, peer->second);
        }
    }
}

void GioNet::Server::Send(const Buffer& buffer, const Peer& peer)
{
    std::optional<int> res = DoSend(buffer, peer);

    if(!res)
    {
        RemovePeer(peer);
    }
}

void GioNet::Server::Stop()
{
    listenSocket->Close();
}

void GioNet::Server::AddPeer(const Peer& peer)
{
    std::unique_lock _{peersMutex};
    
    if(peers.contains(peer.address))
    {
        printf("[ERROR]: Peer with address %s already exists.\n", peer.address.ToString().c_str());
        return;
    }
    
    peers[peer.address] = peer;
    OnPostPeerAdded(peer);
    printf("Successfully connected to peer %s\n", peer.ToString().c_str());
}

void GioNet::Server::RemovePeer(const Peer& peer)
{
    std::unique_lock _{peersMutex};

    auto peerEntry = peers.find(peer.address); 
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