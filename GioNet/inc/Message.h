#pragma once
#include "Buffer.h"

namespace GioNet
{
    struct Message
    {
        bool reliable{true};
        
        Buffer data{};

        Message() = default;

        Message(Buffer&& data);
    };
}
