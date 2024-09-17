#pragma once
#include <cstdint>
#include <optional>

#include "Buffer.h"

namespace GioNet
{
    struct Packet
    {
        using HeaderType = uint8_t;
        using IdType = uint16_t;
        
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

        HeaderType header{0};

        std::optional<IdType> id{};
        
        Buffer payload{};

        static bool HasFlags(HeaderType header, Flags flags);

        Packet() = default;

        Packet(Types type, std::initializer_list<Flags> flags = {}, Buffer&& payload = {});

        Types GetType() const;

        Flags GetFlags() const;

        void SetFlag(Flags flag, bool enabled);

        bool HasFlags(Flags flags) const;

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
