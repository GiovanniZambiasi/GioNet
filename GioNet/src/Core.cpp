#include "Core.h"

#include <cstdio>
#include <format>

std::string GioNet::NetAddress::ToString() const
{
    return std::format("{}:{}", ip, port);
}
