#include "Server.h"
#include <ranges>
#include <string>
#include "Socket.h"

GioNet::Server::Server(unsigned short port)
    : socket(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::UDP))
{
    assert(socket && socket->IsValid());
    socket->Bind();
}

GioNet::Server::~Server()
{
    Server::Stop();
}

void GioNet::Server::Start()
{
    assert(socket && socket->IsValid());
    GIONET_LOG("Starting UDP server...\n");
    listenThread = std::jthread{&Server::ListenThread, this};
    sendThread = std::jthread{&Server::SendThread, this};
}

void GioNet::Server::Stop()
{
    if(socket->IsValid())
    {
        socket->Close();
    }

    if(listenThread.joinable())
    {
        listenThread.request_stop();
    }

    if(sendThread.joinable())
    {
        sendThread.request_stop();
    }
    
    peers.clear();
}

void GioNet::Server::GetPeers(std::vector<std::shared_ptr<Connection>>& outPeers) const
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

int GioNet::Server::GetConnectedPeerCout() const
{
    std::shared_lock _{peersMutex};
    return peers.size();
}

std::shared_ptr<GioNet::Connection> GioNet::Server::TryGetPeer(const NetAddress& address) const
{
    std::shared_lock _{peersMutex};
    auto peer = peers.find(address);

    if(peer != peers.end())
    {
        return peer->second;
    }
    
    return {};
}

bool GioNet::Server::IsRunning() const
{
    return socket && socket->IsValid();
}

void GioNet::Server::Broadcast(const Buffer& buffer, bool reliable, const std::unordered_set<NetAddress>& except)
{
    std::vector<std::shared_ptr<Connection>> peersCopy{};
    GetPeers(peersCopy);

    for (auto peer : peersCopy)
    {
        assert(peer);

        if(except.contains(peer->GetAddress()))
            continue;

        Send(buffer, peer, reliable);
    }
}

void GioNet::Server::Send(const Buffer& buffer, std::shared_ptr<Connection> peer, bool reliable)
{
    peer->Schedule(
        Packet{
            Packet::Types::Data,
            reliable ? Packet::Flags::Reliable : Packet::Flags::None,
            Buffer{buffer}
    });
}

GioNet::Socket& GioNet::Server::GetSocketChecked()
{
    assert(socket && socket->IsValid());
    return *socket;
}

void GioNet::Server::BindDataReceived(DataReceivedDelegate&& delegate)
{
    dataReceivedDelegate = std::move(delegate);
}

void GioNet::Server::BindPeerConnected(ConnectionDelegate&& delegate)
{
    peerConnectedDelegate = std::move(delegate);
}

void GioNet::Server::BindPeerDisconnected(ConnectionDelegate&& delegate)
{
    peerDisconnectedDelegate = std::move(delegate);
}

std::shared_ptr<GioNet::Connection> GioNet::Server::AddPeer(const NetAddress& address)
{
    std::shared_ptr<Connection> peer{}; 
    
    {
        std::unique_lock _{peersMutex};
    
        if(peers.contains(address))
        {
            GIONET_LOG("[ERROR]: Peer with address %s already exists.\n", address.ToString().c_str());
            return {};
        }

        peer = std::make_shared<Connection>(address);
        peers[address] = peer;
    }
    
    if(peerConnectedDelegate)
        peerConnectedDelegate(*peer);
    
    GIONET_LOG("Successfully connected to peer %s\n", peer->ToString().c_str());
    return peer;
}

void GioNet::Server::RemovePeer(std::shared_ptr<Connection> peer)
{
    assert(peer);
    
    auto peerEntry = peers.find(peer->GetAddress()); 
    if(peerEntry != peers.end())
    {
        GIONET_LOG("Disconnecting peer %s\n", peer->ToString().c_str());
        
        {
            std::unique_lock _{peersMutex};
            peers.erase(peerEntry);
        }

        if(peerDisconnectedDelegate)
            peerDisconnectedDelegate(*peer);
    }
    else
    {
        GIONET_LOG("[ERROR]: Tried to remove peer not in connections list: '%s'\n", peer->ToString().c_str());
    }
}

void GioNet::Server::InvokeDataReceived(const Connection& peer, Buffer&& buffer)
{
    if(dataReceivedDelegate)
        dataReceivedDelegate(peer, std::move(buffer));
}

void GioNet::Server::ListenThread()
{
    std::shared_ptr<Socket> socketCopy = GetSocket();
    while (socketCopy && socketCopy->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        NetAddress source{};
        std::optional<Buffer> received = socketCopy->ReceiveFrom(&source);

        if(received)
        {
            std::shared_ptr<Connection> peer = TryGetPeer(source);
            
            if(!peer)
            {
                peer = AddPeer(source);
                
                if(!peer)
                {
                    continue;
                }
            }

            assert(peer);
            Packet packet{received->Read<Packet>()};
            received.reset();
            peer->Received(std::move(packet));
            
            ProcessReceivedPackets();
        }
        else
        {
            // TODO - Fix recv failed after a broken send
        }
    }
}

void GioNet::Server::ProcessReceivedPackets()
{
    for(auto& pair : peers)
    {
        std::shared_ptr<Connection> peer = pair.second;
        assert(peer);

        while (std::optional<Packet> packet = peer->GetReadyIncomingPacket())
        {
            assert(packet);
            Packet& packetRef = *packet;
            
            if(packetRef.GetType() == Packet::Types::Data)
                InvokeDataReceived(*peer, std::move(packet->payload));
        }
    }
}

void GioNet::Server::SendThread()
{
    std::shared_ptr<Socket> socketCopy = GetSocket();
    std::vector<std::shared_ptr<Connection>> connectedPeers{};
    while (socketCopy && socketCopy->IsValid() && !sendThread.get_stop_token().stop_requested())
    {
        GetPeers(connectedPeers);

        for(std::shared_ptr<Connection>& peer : connectedPeers)
        {
            NetAddress address = peer->GetAddress();
            while (std::optional<Packet> outgoingPacket = peer->GetReadyOutgoingPacket())
            {
                Buffer b{};
                b.Write(outgoingPacket.value());
                socketCopy->SendTo(b, address);
                // TODO - Wait a given time to control traffic
            }
        }
        
        connectedPeers.clear();
    }
}
