#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

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
struct mysql_packet_header
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

enum class mysql_session_phase
{
    unknown,
    connection_phase,
    command_phase
};

enum class mysql_connection_phase_step
{
    unknown,
    initial_handshake,
    handshake_response
};

enum class packet_origin
{
    downstream,
    upstream
};

enum class mysql_protocol_version : uint8_t
{
    protocol_handshake_v9 = 0x09,
    protocol_handshake_v10 = 0x0a,
    protocol_unknown
};

template <typename EnumClassType>
constexpr auto to_underlying_type(EnumClassType value)
{
    return static_cast<std::underlying_type<mysql_protocol_version>::type>(value);
}

#pragma pack(push,1)
union capability_flags_t
{
    uint32_t capability_flags;
    struct
    {
        uint8_t cap_flags_00;
        uint8_t cap_flags_01;
        uint8_t cap_flags_02;
        uint8_t cap_flags_03;
    };
    uint32_t operator()() const
    {
        return capability_flags;
    }
};
#pragma pack(pop)

using mysql_status_flags_t = uint16_t;
namespace mysql_status_flags
{
constexpr mysql_status_flags_t server_status_in_trans = 0x0001;
constexpr mysql_status_flags_t server_status_autocommit = 0x0002;
constexpr mysql_status_flags_t server_more_results_exists = 0x0008;
constexpr mysql_status_flags_t server_status_no_good_index_used = 0x0010;
constexpr mysql_status_flags_t server_status_no_index_used = 0x0020;
constexpr mysql_status_flags_t server_status_cursor_exists = 0x0040;
constexpr mysql_status_flags_t server_status_last_row_sent = 0x0080;
constexpr mysql_status_flags_t server_status_db_dropped = 0x0100;
constexpr mysql_status_flags_t server_status_no_backslash_escapes = 0x0200;
constexpr mysql_status_flags_t server_status_metadata_changed = 0x0400;
constexpr mysql_status_flags_t server_query_was_slow = 0x0800;
constexpr mysql_status_flags_t server_ps_out_params = 0x1000;
constexpr mysql_status_flags_t server_status_in_trans_readonly = 0x2000;
constexpr mysql_status_flags_t server_session_state_changed = 0x4000;
}

using mysql_capability_flags_t = uint32_t;
namespace mysql_capability_flags
{
constexpr mysql_capability_flags_t client_long_password = 0x0000'0001;
constexpr mysql_capability_flags_t client_found_rows = 0x0000'0002;
constexpr mysql_capability_flags_t client_long_flag = 0x0000'0004;
constexpr mysql_capability_flags_t client_connect_with_db = 0x0000'0008;
constexpr mysql_capability_flags_t client_no_schema = 0x0000'0010;
constexpr mysql_capability_flags_t client_compress = 0x0000'0020;
constexpr mysql_capability_flags_t client_odbc = 0x0000'0040;
constexpr mysql_capability_flags_t client_local_files = 0x0000'0080;
constexpr mysql_capability_flags_t client_ignore_space = 0x0000'0100;
constexpr mysql_capability_flags_t client_protocol_41 = 0x0000'0200;
constexpr mysql_capability_flags_t client_interactive = 0x0000'0400;
constexpr mysql_capability_flags_t client_ssl = 0x0000'0800;
constexpr mysql_capability_flags_t client_ignore_sigpipe = 0x0000'1000;
constexpr mysql_capability_flags_t client_transactions = 0x0000'2000;
constexpr mysql_capability_flags_t client_reserved = 0x0000'4000;
constexpr mysql_capability_flags_t client_secure_connection = 0x0000'8000;
constexpr mysql_capability_flags_t client_multi_statements = 0x0001'0000;
constexpr mysql_capability_flags_t client_multi_results = 0x0002'0000;
constexpr mysql_capability_flags_t client_ps_multi_results = 0x0004'0000;
constexpr mysql_capability_flags_t client_plugin_auth = 0x0008'0000;
constexpr mysql_capability_flags_t client_connect_attrs = 0x0010'0000;
constexpr mysql_capability_flags_t client_plugin_auth_lenenc_client_data = 0x0020'0000;
constexpr mysql_capability_flags_t client_can_handle_expired_passwords = 0x0040'0000;
constexpr mysql_capability_flags_t client_session_track = 0x0080'0000;
constexpr mysql_capability_flags_t client_deprecate_eof = 0x0100'0000;
}


}
