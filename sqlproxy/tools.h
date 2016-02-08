#pragma once

#include <cstdint>

#ifdef _WIN32
#include <WinSock2.h>
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif // _MSC_VER
#elif defined __linux__
#include <arpa/inet.h>
#endif


namespace sql_proxy
{

inline uint8_t host_to_network_order(const uint8_t value)
{
    return value;
}

inline uint16_t host_to_network_order(const uint16_t value)
{
    return htons(value);
}

inline uint32_t host_to_network_order(const uint32_t value)
{
    return htonl(value);
}

}