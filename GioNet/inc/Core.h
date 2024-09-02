#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define GIONET_DEFAULT_PORT "27015"

#define GIONET_NOCOPY(ClassName) \
    ClassName(const ClassName&) = delete;\
    ClassName& operator=(const ClassName&) = delete;\

#define GIONET_NOMOVE(ClassName)\
    ClassName(ClassName&&) = delete;\
    ClassName& operator=(ClassName&&) = delete;

#define GIONET_NOCOPY_NOMOVE(ClassName)\
    GIONET_NOCOPY(ClassName)\
    GIONET_NOMOVE(ClassName)