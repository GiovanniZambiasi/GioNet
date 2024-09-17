#include "Core.h"

#include <format>

bool GioNet::NetAddress::IsLoopback() const
{
    return ip.starts_with("127") || ip == "localhost";
}

bool GioNet::NetAddress::operator==(const NetAddress& other) const
{
    if(&other == this)
        return true;
    
    if(other.port != port)
        return false;

    if(other.ip == ip)
        return true;

    return IsLoopback() && other.IsLoopback();
}

std::string GioNet::NetAddress::ToString() const
{
    return std::format("{}:{}", ip, port);
}

std::size_t std::hash<GioNet::NetAddress>::operator()(const GioNet::NetAddress& address) const noexcept
{
    return std::hash<std::string>()(address.ToString()) ^ std::hash<unsigned short>()(address.port);
}
