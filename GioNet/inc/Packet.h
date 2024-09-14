#pragma once
#include <cstdint>

#include "Buffer.h"

namespace GioNet
{
    struct Packet
    {
        enum class Type : uint8_t
        {
            Ping = 0,
            Ack,
            Connect,
            Data
        };

        uint16_t header{0};

        Buffer payload{};

        static uint16_t GetMaximumViableId();
        
        Packet() = default;

        Packet(uint16_t id, Type type, Buffer&& payload = {});

        uint16_t GetId() const;
        
        Type GetType() const;

        bool operator==(const Packet& rhs) const = default;

        std::string ToString() const;
    };
}
