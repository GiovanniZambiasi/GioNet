#include "Server.h"
#include <ranges>
#include <string>
#include "Socket.h"

GioNet::Server::Server(unsigned short port)
    : listenSocket(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::UDP))
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
    GIONET_LOG("Starting UDP server...\n");
    listenThread = std::jthread{&Server::RunListenThread, this};
}

void GioNet::Server::Stop()
{
    if(listenSocket->IsValid())
    {
        listenSocket->Close();
    }

    if(listenThread.joinable())
    {
        listenThread.request_stop();
    }
    
    peers.clear();
}

void GioNet::Server::GetPeers(std::vector<Peer>& outPeers) const
{
    std::shared_lock _{peersMutex};
    for (const auto& peer : peers)
    {
        outPeers.push_back(peer.second);
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

bool GioNet::Server::IsRunning() const
{
    return listenSocket && listenSocket->IsValid();
}

void GioNet::Server::Broadcast(const Buffer& buffer, const std::unordered_set<Peer>& except)
{
    std::vector<Peer> peersCopy{};
    GetPeers(peersCopy);

    for (const Peer& peer : peersCopy)
    {
        if(except.contains(peer))
            continue;

        Send(buffer, peer);
    }
}

void GioNet::Server::Send(const Buffer& buffer, const Peer& peer)
{
    std::optional<int> res = GetSocketChecked().SendTo(buffer, peer.address);

    if(!res)
    {
        RemovePeer(peer);
    }
}

GioNet::Socket& GioNet::Server::GetSocketChecked()
{
    assert(listenSocket && listenSocket->IsValid());
    return *listenSocket;
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

void GioNet::Server::RunListenThread()
{
    std::shared_ptr<Socket> socket = GetSocket();
    while (socket && socket->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        NetAddress source{};
        std::optional<Buffer> received = socket->ReceiveFrom(&source);

        if(received)
        {
            if(const Peer* peer = TryGetPeer(source))
            {
                InvokeDataReceived(*peer, std::move(*received));
                received.reset();
            }
            else
            {
                Peer newPeer{source, socket};
                AddPeer(newPeer);
                InvokeDataReceived(newPeer, std::move(*received));
                received.reset();
            }
        }
        else
        {
            // TODO - Fix recv failed after a broken send
        }
    }
}

