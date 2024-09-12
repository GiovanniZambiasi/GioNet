#pragma once

namespace GioNet
{
    class Buffer;
    
    class INetSerializable
    {
    public:
        virtual ~INetSerializable() = default;

        virtual bool Serialize(Buffer& buffer) = 0;

        virtual bool Deserialize(Buffer& buffer) = 0;
        
    protected:
        INetSerializable() = default;
    };
}
