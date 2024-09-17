#pragma once
#include <cassert>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "Buffer.h"
#include "Core.h"
#include "Peer.h"

namespace GioNet
{
    class Socket;
    
    class Server
    {
    public:
        using DataReceivedDelegate = std::function<void(const Peer&, Buffer&&)>;
        
        using PeerDelegate = std::function<void(const Peer&)>;
        
    private:
        std::shared_ptr<Socket> listenSocket{};

        std::jthread listenThread{};
        
        std::unordered_map<NetAddress, Peer> peers{};

        mutable std::shared_mutex peersMutex{};

        DataReceivedDelegate dataReceivedDelegate{};

        PeerDelegate peerConnectedDelegate{};
        
        PeerDelegate peerDisconnectedDelegate{};
        
    public:
        GIONET_NOCOPY(Server)

        Server(unsigned short port);
        
        virtual ~Server();
        
        virtual void Start();

        virtual void Stop();

        void BindDataReceived(DataReceivedDelegate&& delegate);
        
        void BindPeerConnected(PeerDelegate&& delegate);
        
        void BindPeerDisconnected(PeerDelegate&& delegate);
        
        void Broadcast(const Buffer& buffer, const std::unordered_set<Peer>& except = {});

        void Send(const Buffer& buffer, const Peer& peer);

        Socket& GetSocketChecked();

        std::shared_ptr<Socket> GetSocket() { return listenSocket; }

        /**
         * @param outPeers Populates input with list of peers at the time of calling the method. Doesn't return the actual
         * dict for thread safety.
         */
        void GetPeers(std::vector<Peer>& outPeers) const;

        bool HasPeer(const NetAddress& address) const;

        const Peer* TryGetPeer(const NetAddress& address) const;
        
        bool IsRunning() const;

    private:
        void RunListenThread();
        
        void AddPeer(const Peer& peer);

        void RemovePeer(const Peer& peer);

        virtual void OnPostPeerAdded(const Peer& peer) { }

        virtual void OnPrePeerRemoved(const Peer& peer) { }

        void InvokeDataReceived(const Peer& peer, Buffer&& buffer);
    };
}