#pragma once

#include <cstdint>
#include <string>

namespace sql_proxy
{

using host_t = std::string;
using port_t = uint16_t;

struct proxy_server_config_t
{
    host_t local_ip_addr;
    port_t local_port;
    host_t upstream_ip_addr;
    port_t upstream_port;
};

constexpr size_t max_data_length = 8 * 1024; // 8 KiB

}
