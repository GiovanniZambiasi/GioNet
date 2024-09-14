#include "Packet.h"

#include <bitset>
#include <cassert>
#include <format>

namespace
{
    constexpr uint16_t IdMask   = 0b0001111111111000;
    constexpr uint16_t MaxViableId = IdMask >> 3;
    constexpr uint16_t TypeMask = 0b0000000000000111;
    constexpr uint16_t MaxViableType = TypeMask;
}

uint16_t GioNet::Packet::GetMaximumViableId()
{
    return MaxViableId;
}

GioNet::Packet::Packet(uint16_t id, Type type, Buffer&& payload)
    :payload(std::move(payload))
{
    assert(id <= MaxViableId);
    header = header | (id << 3);

    uint8_t typeBits = static_cast<uint8_t>(type);
    assert(typeBits <= MaxViableType);
    header |= typeBits;
}

uint16_t GioNet::Packet::GetId() const
{
    uint16_t id = header & IdMask;
    id = id >> 3;
    return id;
}

GioNet::Packet::Type GioNet::Packet::GetType() const
{
    uint8_t typeBits = header & TypeMask;
    return static_cast<Type>(typeBits);
}

std::string GioNet::Packet::ToString() const
{
    return std::format("(Packet: id {} | type {} | payload {} bytes)", GetId(), static_cast<unsigned char>(GetType()), payload.Length());
}
