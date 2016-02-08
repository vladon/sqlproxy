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
    result += "Protocol Version = " + std::to_string(protocol_version);

    return result;
}

}
