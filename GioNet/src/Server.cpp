#include "Server.h"
#include <ranges>
#include <string>
#include "Socket.h"

std::string GioNet::Peer::ToString() const
{
    return connection ? connection->ToString() : address.ToString();
}

void GioNet::Server::GetPeers(std::unordered_map<NetAddress, Peer>& outPeers) const
{
    std::shared_lock _{peersMutex};
    for (const auto& peer : peers)
    {
        outPeers[peer.first] = peer.second;
    }
}

bool GioNet::Server::HasPeer(const NetAddress& address) const
{
    std::shared_lock _{peersMutex};
    return peers.contains(address);
}

const GioNet::Peer* GioNet::Server::TryGetPeer(const NetAddress& address) const
{
    std::shared_lock _{peersMutex};
    auto peer = peers.find(address);

    if(peer != peers.end())
    {
        return &peer->second;
    }
    
    return nullptr;
}

GioNet::Server::Server(const std::shared_ptr<Socket>& listenSocket)
    : listenSocket(listenSocket)
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Bind();
}

GioNet::Server::~Server()
{
    Server::Stop();
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
        const Peer* peer = TryGetPeer(address);

        if(peer != nullptr)
        {
            Send(buffer, *peer);
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
    if(listenSocket->IsValid())
    {
        listenSocket->Close();
    }
    
    peers.clear();
}

void GioNet::Server::BindDataReceived(DataReceivedDelegate&& delegate)
{
    dataReceivedDelegate = std::move(delegate);
}

void GioNet::Server::BindPeerConnected(PeerDelegate&& delegate)
{
    peerConnectedDelegate = std::move(delegate);
}

void GioNet::Server::BindPeerDisconnected(PeerDelegate&& delegate)
{
    peerDisconnectedDelegate = std::move(delegate);
}

void GioNet::Server::AddPeer(const Peer& peer)
{
    {
        std::unique_lock _{peersMutex};
    
        if(peers.contains(peer.address))
        {
            GIONET_LOG("[ERROR]: Peer with address %s already exists.\n", peer.address.ToString().c_str());
            return;
        }
    
        peers[peer.address] = peer;
    }
    
    OnPostPeerAdded(peer);

    if(peerConnectedDelegate)
        peerConnectedDelegate(peer);
    
    GIONET_LOG("Successfully connected to peer %s\n", peer.ToString().c_str());
}

void GioNet::Server::RemovePeer(const Peer& peer)
{
    auto peerEntry = peers.find(peer.address); 
    if(peerEntry != peers.end())
    {
        GIONET_LOG("Disconnecting peer %s\n", peer.ToString().c_str());
        OnPrePeerRemoved(peer);
        
        {
            std::unique_lock _{peersMutex};
            peers.erase(peerEntry);
        }

        if(peerDisconnectedDelegate)
            peerDisconnectedDelegate(peer);
    }
    else
    {
        GIONET_LOG("[ERROR]: Tried to remove peer not in connections list: '%s'\n", peer.ToString().c_str());
    }
}

void GioNet::Server::InvokeDataReceived(const Peer& peer, Buffer&& buffer)
{
    if(dataReceivedDelegate)
        dataReceivedDelegate(peer, std::move(buffer));
}
