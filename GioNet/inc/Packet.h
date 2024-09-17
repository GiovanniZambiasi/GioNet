#pragma once
#include <cstdint>

#include "Buffer.h"

namespace GioNet
{
    struct Packet
    {
        enum class Types : uint8_t
        {
            Ping = 0,
            Ack,
            Connect,
            Data
        };

        enum Flags : uint8_t
        {
            Reliable = 0b1,
            Fragmented = 0b10,
        };

        uint8_t header{0};

        Buffer payload{};

        Packet() = default;

        Packet(Types type, std::initializer_list<Flags> flags = {}, Buffer&& payload = {});

        Types GetType() const;

        Flags GetFlags() const;

        void SetFlag(Flags flag, bool enabled);

        bool HasFlag(Flags flag) const;

        bool operator==(const Packet& rhs) const = default;

        std::string ToString() const;
    };

    inline Packet::Flags operator|(Packet::Flags lhs, Packet::Flags rhs)
    {
        return static_cast<Packet::Flags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    inline Packet::Flags& operator|=(Packet::Flags& lhs, Packet::Flags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

}
