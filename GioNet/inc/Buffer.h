#pragma once
#include <string_view>
#include <vector>

namespace GioNet
{
    class Buffer
    {
        std::vector<char> payload{};
    public:
        Buffer() = default;

        Buffer(const char* data, int length);

        Buffer(std::string_view sv);

        const char* Data() const { return payload.data(); }
        
        int Length() const { return static_cast<int>(payload.size()); }
    };
}
