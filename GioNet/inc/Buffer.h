#pragma once
#include <cassert>
#include <string_view>
#include <vector>

#define DECLARE_WRITE_SPEC(Type) template<> void GioNet::Buffer::Write(const Type& val);

#define DECLARE_READ_SPEC(Type) template<> Type GioNet::Buffer::Read();

#define DECLARE_READ_WRITE_SPEC(Type)\
    DECLARE_WRITE_SPEC(Type)\
    DECLARE_READ_SPEC(Type)

namespace GioNet
{
    struct Packet;
    
    /**
     * Network serializable buffer
     */
    class Buffer
    {
        std::vector<int8_t> payload{};
        
    public:
        Buffer() = default;

        /**
         * Constructs the buffer from raw data 
         */
        Buffer(const void* data, int length);

        Buffer(std::string_view view);

        Buffer(const Packet& packet);

        const int8_t* Data() const { return payload.data(); }
        
        int32_t Length() const { return static_cast<int32_t>(payload.size()); }

        bool IsEmpty() const { return payload.empty(); }

        bool operator==(const Buffer& rhs) const;
        
        template<typename T>
        void Write(const T& value)
        {
            static_assert(std::is_trivially_copyable_v<T>, "T is a complex type, and must define a specialization for Write");
            WriteBytes(value);
        }

        template<typename T>
        void WriteBytes(const T& value)
        {
            // TODO - Who cares about endianness anyway?
            CopyBytesIntoPayload(&value, sizeof(T));
        }

        template<typename T>
        T Read()
        {
            // TODO - Who cares about endianness anyway?
            static_assert(std::is_default_constructible_v<T>, "T must be default constructible, or define a specialization of this function");
            assert(Length() >= sizeof(T));
            T v{};
            ExtractBytesFromPayload(&v, sizeof(T));
            return v;
        }

        /**
         * Copies payload from other buffer without removing existing data in this buffer 
         */
        void Copy(const Buffer& other);

    private:
        void CopyBytesIntoPayload(const void* src, size_t size);

        void ExtractBytesFromPayload(void* dest, size_t size);
    };
}

namespace GioNet
{
    struct Packet;
}

DECLARE_READ_WRITE_SPEC(std::string)

DECLARE_WRITE_SPEC(std::string_view)

DECLARE_READ_WRITE_SPEC(GioNet::Buffer)

DECLARE_READ_WRITE_SPEC(GioNet::Packet)
