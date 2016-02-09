#include "common.h"
#include "mysql_parser.h"

namespace sql_proxy
{

std::string mysql_parser::parse_packet(const bytes_t & data, const packet_origin origin)
{
    std::string result;

    auto header = reinterpret_cast<const mysql_packet_header *>(data.data());
    auto payload_length = header->get_payload_length();
    result += "Payload length: " + std::to_string(payload_length) + "\n";

    auto sequence_id = header->get_sequence_id();
    result += "Sequence ID: " + std::to_string(sequence_id) + "\n";

    switch (phase_)
    {
        case mysql_session_phase::connection_phase:
            result += parse_connection_phase_packet(data, origin);
            break;
        case mysql_session_phase::command_phase:
            result += parse_command_phase_packet(data, origin);
            break;
        case mysql_session_phase::unknown:
            throw std::domain_error("Incorrect mysql session phase");
    }

    return result;
}

std::string mysql_parser::parse_connection_phase_packet(const bytes_t & data, const packet_origin origin)
{
    std::string result;

    if (origin == packet_origin::upstream)
    {
        if (connection_step_ == mysql_connection_phase_step::initial_handshake)
        {
            result += parse_initial_handshake(data);
        }
    }

    return result;
}

std::string mysql_parser::parse_command_phase_packet(const bytes_t & data, const packet_origin origin)
{
    std::string result;

    return result;
}

std::string mysql_parser::parse_initial_handshake(const bytes_t & data)
{
    std::string result;

    uint8_t protocol_version = *(data.data() + sizeof(mysql_packet_header));
    result += "Protocol Version = " + std::to_string(protocol_version) + "\n";
    switch (protocol_version)
    {
        case to_underlying_type(mysql_protocol_version::protocol_handshake_v10):
            result += parse_handshake_v10(data);
            break;
        default:
            result += "Protocol not supported in the parser\n";
            break;
    }
    connection_step_ = mysql_connection_phase_step::handshake_response;

    return result;
}

std::string mysql_parser::parse_handshake_v10(const bytes_t & data)
{
    std::string result;

    size_t current_offset = 5;

    auto server_version = std::string(reinterpret_cast<const char*>(data.data() + current_offset));
    result += "Server version: " + server_version + "\n";
    current_offset += server_version.size() + 1;

    auto connection_id = *reinterpret_cast<const uint32_t *>(data.data() + current_offset);
    result += "Connection ID: " + std::to_string(connection_id) + "\n";
    current_offset += sizeof(connection_id);

    auto auth_plugin_data = std::string(reinterpret_cast<const char *>(data.data() + current_offset), 8);
    current_offset += auth_plugin_data.size();
    current_offset += 1; // skip filler

    capability_flags_t capability_flags{};
    capability_flags.cap_flags_00 = *(data.data() + current_offset);
    current_offset += sizeof(capability_flags.cap_flags_00);
    capability_flags.cap_flags_01 = *(data.data() + current_offset);
    current_offset += sizeof(capability_flags.cap_flags_01);

    if (current_offset < data.size())
    {
        auto character_set = *(data.data() + current_offset);
        result += "Default character set: " + std::to_string(character_set) + "\n";
        current_offset += sizeof(character_set);

        auto status_flags = *reinterpret_cast<const mysql_status_flags_t *>(data.data() + current_offset);
        result += "Status flags: " + std::to_string(status_flags) + ":\n"
            + parse_status_flags(status_flags) + "\n";
        current_offset += sizeof(status_flags);

        capability_flags.cap_flags_02 = *(data.data() + current_offset);
        current_offset += sizeof(capability_flags.cap_flags_02);
        capability_flags.cap_flags_03 = *(data.data() + current_offset);
        current_offset += sizeof(capability_flags.cap_flags_03);

        size_t auth_plugin_data_length{ 0 };
        if (capability_flags() & mysql_capability_flags::client_plugin_auth)
        {
            auth_plugin_data_length = *reinterpret_cast<const uint8_t *>(data.data() + current_offset);
        }
        current_offset += 1;

        current_offset += 10; // skip reserved 10 times [0x00]
        
        if (capability_flags() & mysql_capability_flags::client_secure_connection)
        {
            auto auth_plugin_data_part2_length = std::max(std::size_t{ 13 }, auth_plugin_data_length - 8);
            auth_plugin_data += std::string(reinterpret_cast<const char *>(data.data() + current_offset), auth_plugin_data_part2_length);
            current_offset += auth_plugin_data_part2_length;
        }

        if (capability_flags() & mysql_capability_flags::client_plugin_auth)
        {
            auto auth_plugin_name = std::string(reinterpret_cast<const char *>(data.data() + current_offset));
            result += "Auth plugin name: " + auth_plugin_name + "\n";
        }
    }
    result += "Capability flags: " + std::to_string(capability_flags()) + ":\n" +
        parse_capability_flags(capability_flags) + "\n";

    return result;
}

std::string mysql_parser::parse_status_flags(const mysql_status_flags_t sf)
{
    std::string result;

    namespace msf = mysql_status_flags;

    if (sf & msf::server_status_in_trans) result += "\tserver_status_in_trans\n";
    if (sf & msf::server_status_autocommit) result += "\tserver_status_autocommit\n";
    if (sf & msf::server_more_results_exists) result += "\tserver_more_results_exists\n";
    if (sf & msf::server_status_no_good_index_used) result += "\tserver_status_no_good_index_used\n";
    if (sf & msf::server_status_no_index_used) result += "\tserver_status_no_index_used\n";
    if (sf & msf::server_status_cursor_exists) result += "\tserver_status_cursor_exists\n";
    if (sf & msf::server_status_last_row_sent) result += "\tserver_status_last_row_sent\n";
    if (sf & msf::server_status_db_dropped) result += "\tserver_status_db_dropped\n";
    if (sf & msf::server_status_no_backslash_escapes) result += "\tserver_status_no_backslash_escapes\n";
    if (sf & msf::server_status_metadata_changed) result += "\tserver_status_metadata_changed\n";
    if (sf & msf::server_query_was_slow) result += "\tserver_query_was_slow\n";
    if (sf & msf::server_ps_out_params) result += "\tserver_ps_out_params\n";
    if (sf & msf::server_status_in_trans_readonly) result += "\tserver_status_in_trans_readonly\n";
    if (sf & msf::server_session_state_changed) result += "\tserver_session_state_changed\n";

    return result;
}

std::string mysql_parser::parse_capability_flags(const capability_flags_t capability_flags)
{
    std::string result;

    auto cf = capability_flags();

    namespace mcf = mysql_capability_flags;

    if (cf & mcf::client_long_password) result += "\tclient_long_password\n";
    if (cf & mcf::client_found_rows) result += "\tclient_found_rows\n";
    if (cf & mcf::client_long_flag) result += "\tclient_long_flag\n";
    if (cf & mcf::client_connect_with_db) result += "\tclient_connect_with_db\n";
    if (cf & mcf::client_no_schema) result += "\tclient_no_schema\n";
    if (cf & mcf::client_compress) result += "\tclient_compress\n";
    if (cf & mcf::client_odbc) result += "\tclient_odbc\n";
    if (cf & mcf::client_local_files) result += "\tclient_local_files\n";
    if (cf & mcf::client_ignore_space) result += "\tclient_ignore_space\n";
    if (cf & mcf::client_protocol_41) result += "\tclient_protocol_41\n";
    if (cf & mcf::client_interactive) result += "\tclient_interactive\n";
    if (cf & mcf::client_ssl) result += "\tclient_ssl\n";
    if (cf & mcf::client_ignore_sigpipe) result += "\tclient_ignore_sigpipe\n";
    if (cf & mcf::client_transactions) result += "\tclient_transactions\n";
    if (cf & mcf::client_reserved) result += "\tclient_reserved\n";
    if (cf & mcf::client_secure_connection) result += "\tclient_secure_connection\n";
    if (cf & mcf::client_multi_statements) result += "\tclient_multi_statements\n";
    if (cf & mcf::client_multi_results) result += "\tclient_multi_results\n";
    if (cf & mcf::client_ps_multi_results) result += "\tclient_ps_multi_results\n";
    if (cf & mcf::client_plugin_auth) result += "\tclient_plugin_auth\n";
    if (cf & mcf::client_connect_attrs) result += "\tclient_connect_attrs\n";
    if (cf & mcf::client_plugin_auth_lenenc_client_data) result += "\tclient_plugin_auth_lenenc_client_data\n";
    if (cf & mcf::client_can_handle_expired_passwords) result += "\tclient_can_handle_expired_passwords\n";
    if (cf & mcf::client_session_track) result += "\tclient_session_track\n";
    if (cf & mcf::client_deprecate_eof) result += "\tclient_deprecate_eof\n";

    return result;
}
}
