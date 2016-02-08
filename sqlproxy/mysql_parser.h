#pragma once

#include "common.h"

namespace sql_proxy
{

class mysql_parser
{
public:
    static std::string parse_packet(const bytes_t & data);
};

}
