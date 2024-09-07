#pragma once
#include <cassert>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "Buffer.h"
#include "Core.h"
#include "Socket.h"

namespace GioNet
{
    enum class CommunicationProtocols;
    class Socket;

    struct Peer
    {
        NetAddress address{};
        
        std::shared_ptr<Socket> connection{};
        
        std::string ToString() const;

        bool operator==(const Peer& other) const = default;
    };

    class Server
    {
    public:
        using DataReceivedDelegate = std::function<void(const Peer&, Buffer&&)>;
        
    private:
        std::shared_ptr<Socket> listenSocket{};

        std::unordered_map<NetAddress, Peer> peers{};

        mutable std::shared_mutex peersMutex{};

        DataReceivedDelegate dataReceived{};
        
    public:
        GIONET_NOCOPY(Server)
        
        virtual ~Server();
        
        virtual void Start();

        virtual void Stop();

        void BindDataReceived(DataReceivedDelegate&& delegate);
        
        void Broadcast(const Buffer& buffer);

        void Send(const Buffer& buffer, const Peer& peer);

        Socket& GetSocketChecked()
        {
            assert(listenSocket && listenSocket->IsValid());
            return *listenSocket;
        }

        std::shared_ptr<Socket> GetSocket() { return listenSocket; }

        /**
         * @param outPeers Populates input with list of peers at the time of calling the method. Doesn't return the actual
         * dict for thread safety.
         */
        void GetPeers(std::unordered_map<NetAddress, Peer>& outPeers) const;

        bool HasPeer(const NetAddress& address) const;

        const Peer* TryGetPeer(const NetAddress& address) const;
        
        bool IsRunning() const { return listenSocket && listenSocket->IsValid(); }

    protected:
        Server(const std::shared_ptr<Socket>& listenSocket);

        void AddPeer(const Peer& peer);

        void RemovePeer(const Peer& peer);
        
        virtual void OnPostPeerAdded(const Peer& peer) { }

        virtual void OnPrePeerRemoved(const Peer& peer) { }

        virtual std::optional<int> DoSend(const Buffer& buffer, const Peer& peer) = 0;

        void InvokeDataReceived(const Peer& peer, Buffer&& buffer);

    };
}
