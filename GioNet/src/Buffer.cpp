#include "Buffer.h"
#include <cassert>
#include "Core.h"
#include "Packet.h"

GioNet::Buffer::Buffer(const void* data, int length)
{
    assert(length <= GIONET_BUFFER_MAX);
    payload = std::vector<int8_t>{};
    CopyBytesIntoPayload(data, length);
}

GioNet::Buffer::Buffer(std::string_view view)
{
    Write(view);
}

GioNet::Buffer::Buffer(const Packet& packet)
{
    Write(packet);
}

bool GioNet::Buffer::operator==(const Buffer& rhs) const
{
    if(Length() != rhs.Length())
        return false;

    return std::equal(payload.begin(), payload.end(), rhs.payload.begin(), rhs.payload.end());
}

void GioNet::Buffer::Copy(const Buffer& other)
{
    payload.insert(payload.end(), other.payload.begin(), other.payload.end());
}

void GioNet::Buffer::CopyBytesIntoPayload(const void* src, size_t size)
{
    if(size == 0)
        return;
    
    size_t beforeResize = payload.size();
    payload.resize(payload.size() + size, 0);
    int8_t* dataHead = payload.data() + beforeResize;
    memcpy(dataHead, src, size);
}

void GioNet::Buffer::ExtractBytesFromPayload(void* dest, size_t size)
{
    if(size == 0)
        return;
    
    assert(payload.size() >= size);
    memcpy(dest, payload.data(), size);
    payload.erase(payload.begin(), payload.begin() + size);
}

template<>
void GioNet::Buffer::Write(const std::string& val)
{
    int32_t length = static_cast<int32_t>(val.length());
    Write(length);
    // Not copying termination char because it will be added implicitly by std::string on deserialize
    CopyBytesIntoPayload(val.data(), val.length()); 
}

template<>
std::string GioNet::Buffer::Read()
{
    int32_t length = Read<int32_t>();
    std::string result{};

    if(length > 0)
    {
        result.resize(length, 0);
        ExtractBytesFromPayload(result.data(), length);
    }
    
    return result;
}

template<>
void GioNet::Buffer::Write(const std::string_view& val)
{
    int32_t length = static_cast<int32_t>(val.length());
    Write(length);
    // Not copying termination char because it will be added implicitly by std::string on deserialize
    CopyBytesIntoPayload(val.data(), val.length()); 
}

template<>
void GioNet::Buffer::Write(const Buffer& val)
{
    int32_t length = val.Length();
    Write(length);
    CopyBytesIntoPayload(val.Data(), val.Length()); 
}

template<>
GioNet::Buffer GioNet::Buffer::Read()
{
    int32_t length = Read<int32_t>();

    Buffer result{};

    if(length > 0)
    {
        result.payload.resize(length, 0);
        ExtractBytesFromPayload(result.payload.data(), length);
    }
    
    return result;
}

template<>
void GioNet::Buffer::Write(const Packet& val)
{
    Write<Packet::HeaderType>(val.header);
    Write<Packet::AckHeaderType>(val.ack);
    Write<Packet::IdType>(val.id);
    Write(val.payload);
}

template<>
GioNet::Packet GioNet::Buffer::Read()
{
    Packet packet{};
    packet.header = Read<Packet::HeaderType>();
    packet.ack = Read<Packet::AckHeaderType>();
    packet.id = Read<Packet::IdType>();
    packet.payload = Read<Buffer>();
    return packet;
}