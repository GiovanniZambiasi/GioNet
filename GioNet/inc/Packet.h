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
        using AckBitsetType = uint32_t;

        constexpr static IdType InvalidId = 0;
        constexpr static IdType MaxPossibleId = ~0;
        
        enum class Types : uint8_t
        {
            Connect = 0,
            Data,
            Ping,
        };

        enum class Flags : uint8_t
        {
            None = 0,
            Reliable = 0b1,
        };

        HeaderType header{0};
        
        IdType id{InvalidId};

        IdType ackId{InvalidId};
        
        AckBitsetType ackBitset{0};
        
        Buffer payload{};

        static bool HasFlags(HeaderType header, Flags flags);

        Packet() = default;

        Packet(Types type, Flags flags = Flags::None, Buffer&& payload = {});

        Types GetType() const;

        Flags GetFlags() const;

        void SetFlags(Flags flag, bool enabled);

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
