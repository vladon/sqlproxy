#pragma once

#include <cstdint>
#include <string>

#include "tools.h"

namespace sql_proxy
{

using host_t = std::string;
using port_t = uint16_t;

using byte_t = uint8_t;
using bytes_t = std::vector<byte_t>;

inline std::string to_hex_string(byte_t byte)
{
    std::string result{ "00" };

    auto to_hex = [](byte_t b) -> char
    {
        if (b <= 9)
        {
            return '0' + b;
        }
        else
        {
            return 'A' + (b - 0x0A);
        }
    };

    result[0] = to_hex(byte / 0x10);
    result[1] = to_hex(byte % 0x10);
    return result;
}

template <typename T>
inline std::string to_hex_string(T value)
{
    std::string result;
    result.reserve(sizeof(value) * 2);

    value = host_to_network_order(value);

    auto value_as_bytes = reinterpret_cast<uint8_t *>(&value);

    for (size_t i = 0; i < sizeof(value); ++i)
    {
        result += to_hex_string(value_as_bytes[i]);
    }

    return result;
}

struct proxy_server_config_t
{
    host_t local_ip_addr;
    port_t local_port;
    host_t upstream_ip_addr;
    port_t upstream_port;
};

constexpr size_t max_data_length = 8 * 1024; // 8 KiB

// error handler

class IErrorHandler
{
public:
    virtual ~IErrorHandler() = 0;
};

inline IErrorHandler::~IErrorHandler()
{
}

// data monitor
class IProvider
{
public:
    virtual void on_connect() const = 0;
    virtual void on_downstream_read(bytes_t data) = 0;
    virtual void on_upstream_read(bytes_t data) = 0;

    virtual ~IProvider() = 0;
};

inline IProvider::~IProvider()
{
}

// mysql common
#pragma pack(push, 1)
struct mysql_header
{
    uint8_t payload_length_00;
    uint8_t payload_length_01;
    uint8_t payload_length_02;
    uint8_t sequence_id;

    size_t get_payload_length() const
    {
        return payload_length_00 + payload_length_01 * 0x100 + payload_length_02 * 0x10000;
    }

    uint8_t get_sequence_id() const
    {
        return sequence_id;
    }
};
#pragma pack(pop)

}
