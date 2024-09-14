#pragma once
#include <string_view>
#include <vector>

#define DECLARE_WRITE_SPEC(Type) template<> void GioNet::Buffer::Write(const Type& val);

#define DECLARE_READ_SPEC(Type) template<> Type GioNet::Buffer::ReadBytesAndConstruct();

#define DECLARE_READ_WRITE_SPEC(Type)\
    DECLARE_WRITE_SPEC(Type)\
    DECLARE_READ_SPEC(Type)

namespace GioNet
{
    class Buffer
    {
        std::vector<char> payload{};
        
    public:
        Buffer() = default;

        /**
         * Constructs the buffer from raw data 
         */
        Buffer(const void* data, int length);

        Buffer(std::string_view view);

        const char* Data() const { return payload.data(); }
        
        int Length() const { return static_cast<int>(payload.size()); }

        bool IsEmpty() const { return payload.empty(); }

        bool operator==(const Buffer& rhs) const;
        
        template<typename T>
        void Write(const T& value);

        template<typename T>
        void WriteBytes(const T& value)
        {
            // TODO - Who cares about endianness anyway?
            payload.resize(payload.size() + sizeof(T), 0);
            char* data = payload.data();
            memcpy(data, &value, sizeof(T));
        }

        template<typename T>
        T ReadBytesAndConstruct()
        {
            // TODO - Who cares about endianness anyway?
            static_assert(std::is_default_constructible_v<T>, "T must be default constructible, or define a specialization of this function");
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

DECLARE_WRITE_SPEC(int)

DECLARE_WRITE_SPEC(char)

DECLARE_READ_WRITE_SPEC(std::string)

DECLARE_WRITE_SPEC(std::string_view)

DECLARE_READ_WRITE_SPEC(GioNet::Buffer)