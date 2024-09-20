#include "Packet.h"

#include <bitset>
#include <cassert>
#include <format>

#include "Core.h"

namespace
{
    constexpr uint8_t TypeMask     = 0b00001111;
    constexpr uint8_t TypeOffset   = 0;
    constexpr uint8_t MaxViableType = TypeMask >> TypeOffset;
    
    constexpr uint8_t FlagsMask    = 0b11110000;
    constexpr uint8_t FlagsOffset  = 4;
    constexpr uint8_t MaxViableFlags = FlagsMask >> FlagsOffset;
}

bool GioNet::Packet::HasFlags(HeaderType header, Flags flags)
{
    uint8_t flagBits = static_cast<uint8_t>(flags);
    assert(flagBits <= MaxViableFlags);
    return (header & (flagBits << FlagsOffset)) != 0;
}

GioNet::Packet::Packet(Types type,  Flags flags, Buffer&& payload)
    : payload(std::move(payload))
{
    uint8_t typeBits = static_cast<uint8_t>(type);
    assert(typeBits <= MaxViableType);
    header |= typeBits << TypeOffset;

    SetFlags(flags, true);

    if(type == Types::Ping && payload.Length() > 0)
    {
        GIONET_LOG("[ERROR]: Packet type set to ping, but payload length is not zero. Data will not be serialized");
    }
}

GioNet::Packet::Types GioNet::Packet::GetType() const
{
    uint8_t typeBits = (header & TypeMask) >> TypeOffset;
    return static_cast<Types>(typeBits);
}

GioNet::Packet::Flags GioNet::Packet::GetFlags() const
{
    uint8_t flagsBits = (header & FlagsMask) >> FlagsOffset;
    return static_cast<Flags>(flagsBits);
}

void GioNet::Packet::SetFlags(Flags flag, bool enabled)
{
    uint8_t flagBits = static_cast<uint8_t>(flag);
    assert(flagBits <= MaxViableFlags);
    
    if(enabled)
    {
        header |= flagBits << FlagsOffset;
    }
    else
    {
        header &= ~flagBits;
    }
}

bool GioNet::Packet::HasFlags(Flags flags) const
{
    return HasFlags(header, flags);
}

std::string GioNet::Packet::ToString() const
{
    return std::format("(Packet: header {} | payload {} bytes)", std::bitset<sizeof(header) * 8>{header}.to_string(),
        static_cast<unsigned char>(GetType()), payload.Length());
}
