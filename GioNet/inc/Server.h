#pragma once
#include <cassert>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "Buffer.h"
#include "Core.h"
#include "Connection.h"

namespace GioNet
{
    class Socket;
    
    class Server
    {
    public:
        using DataReceivedDelegate = std::function<void(const Connection&, Buffer&&)>;
        
        using ConnectionDelegate = std::function<void(const Connection&)>;
        
    private:
        std::shared_ptr<Socket> socket{};

        std::jthread listenThread{};
        
        std::jthread sendThread{};
        
        std::unordered_map<NetAddress, std::shared_ptr<Connection>> peers{};

        mutable std::shared_mutex peersMutex{};

        DataReceivedDelegate dataReceivedDelegate{};

        ConnectionDelegate peerConnectedDelegate{};
        
        ConnectionDelegate peerDisconnectedDelegate{};
        
    public:
        GIONET_NOCOPY(Server)

        Server(unsigned short port);
        
        virtual ~Server();
        
        virtual void Start();

        virtual void Stop();

        void BindDataReceived(DataReceivedDelegate&& delegate);
        
        void BindPeerConnected(ConnectionDelegate&& delegate);
        
        void BindPeerDisconnected(ConnectionDelegate&& delegate);
        
        void Broadcast(const Buffer& buffer, bool reliable = true, const std::unordered_set<NetAddress>& except = {});

        void Send(const Buffer& buffer, std::shared_ptr<Connection> peer, bool reliable = true);

        Socket& GetSocketChecked();

        std::shared_ptr<Socket> GetSocket() { return socket; }

        /**
         * @param outPeers Populates input with list of peers at the time of calling the method. Doesn't return the actual
         * dict for thread safety.
         */
        void GetPeers(std::vector<std::shared_ptr<Connection>>& outPeers) const;

        bool HasPeer(const NetAddress& address) const;

        int GetConnectedPeerCout() const;

        std::shared_ptr<Connection> TryGetPeer(const NetAddress& address) const;
        
        bool IsRunning() const;

    private:
        void ListenThread();

        void ProcessReceivedPackets();

        void SendThread();
        
        std::shared_ptr<Connection> AddPeer(const NetAddress& address);

        void RemovePeer(std::shared_ptr<Connection> peer);

        void InvokeDataReceived(const Connection& peer, Buffer&& buffer);
    };
}