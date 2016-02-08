#include "mysql_parser.h"

namespace sql_proxy
{

std::string mysql_parser::parse_packet(const bytes_t & data)
{
    std::string result;
    
    if (data.size() < 5)
    {
        result = "ERROR: Invalid packet";
    }
    else
    {
        size_t payload_length = data[0] + data[1] * 0x100 + data[2] * 0x10000;
        result += "Payload length: " + std::to_string(payload_length) + "\n";

        auto sequence_id = data[3];
        result += "Sequence ID: " + std::to_string(sequence_id) + "\n";
    }

    return result;
}

}
