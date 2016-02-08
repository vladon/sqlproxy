#pragma once

#include "common.h"

namespace sql_proxy
{

class mysql_parser
{
public:
    std::string parse_packet(const bytes_t & data, const packet_origin origin);

private:
    std::string parse_connection_phase_packet(const bytes_t & data, const packet_origin origin);
    std::string parse_command_phase_packet(const bytes_t & data, const packet_origin origin);

    std::string parse_initial_handshake(const bytes_t & data);

    mysql_session_phase phase_{ mysql_session_phase::connection_phase };
    mysql_connection_phase_step connection_step_{ mysql_connection_phase_step::initial_handshake };
};

}
