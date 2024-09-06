#pragma once
#include <cassert>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

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
        std::shared_ptr<Socket> listenSocket{};

        std::vector<Peer> peers{};

        std::shared_mutex peersMutex{};

    public:
        GIONET_NOCOPY(Server)
        
        virtual ~Server();
        
        virtual void Start();

        void Stop();

        Socket& GetSocketChecked()
        {
            assert(listenSocket && listenSocket->IsValid());
            return *listenSocket;
        }

        std::shared_ptr<Socket> GetSocket() { return listenSocket; }
 
    protected:
        Server(const std::shared_ptr<Socket>& listenSocket);

        void AddPeer(const Peer& peer);

        void RemovePeer(const Peer& peer);
        
        virtual void OnPostPeerAdded(const Peer& peer) { }

        virtual void OnPrePeerRemoved(const Peer& peer) { }

    };
}
