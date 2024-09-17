#include "Packet.h"

#include <bitset>
#include <cassert>
#include <format>

namespace
{
    constexpr uint8_t TypeMask     = 0b00001111;
    constexpr uint8_t TypeOffset   = 0;
    constexpr uint8_t MaxViableType = TypeMask >> TypeOffset;
    
    constexpr uint8_t FlagsMask    = 0b11110000;
    constexpr uint8_t FlagsOffset  = 4;
    constexpr uint8_t MaxViableFlags = FlagsMask >> FlagsOffset;
}

GioNet::Packet::Packet(Types type, std::initializer_list<Flags> flags, Buffer&& payload)
    :payload(std::move(payload))
{
    uint8_t typeBits = static_cast<uint8_t>(type);
    assert(typeBits <= MaxViableType);
    header |= typeBits << TypeOffset;

    Flags packedFlags{0};
    for (Flags flag : flags)
    {
        packedFlags |= flag;
    }
    SetFlag(packedFlags, true);
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

void GioNet::Packet::SetFlag(Flags flag, bool enabled)
{
    uint8_t flagBits = flag;
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

bool GioNet::Packet::HasFlag(Flags flag) const
{
    uint8_t flagBits = flag;
    assert(flagBits <= MaxViableFlags);
    return (header & (flagBits << FlagsOffset)) != 0;
}

std::string GioNet::Packet::ToString() const
{
    return std::format("(Packet: header {} | payload {} bytes)", std::bitset<sizeof(uint8_t)>{header}.to_string(),
        static_cast<unsigned char>(GetType()), payload.Length());
}
