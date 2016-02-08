#include "stdafx.h"

#include "mysql_monitor.h"

#include <boost/detail/endian.hpp>

namespace sql_proxy
{

constexpr size_t mysql_packet_header_size = 4;

mysql_packet_header parse_mysql_packet_header(const std::vector<byte_t> & packet)
{
    mysql_packet_header result{};
    if (packet.size() >= mysql_packet_header_size)
    {
        result.payload_length = packet[0] + (packet[1] * 0x100) + (packet[2] * 0x10000);
        result.sequence_id = packet[3];
        result.filled = true;
    }

    return result;
}

mysql_monitor::mysql_monitor(boost::asio::io_service& io_service)
    :
    io_service_(io_service),
    strand_(io_service)
{
}

void mysql_monitor::on_connect() const
{
}

void mysql_monitor::on_downstream_read(bytes_t data)
{
    std::lock_guard<std::mutex> downstream_read_lock(downstream_read_mutex_);

    on_read(data, read_direction::downstream);
}

void mysql_monitor::on_upstream_read(bytes_t data)
{
    std::lock_guard<std::mutex> upstream_read_lock(upstream_read_mutex_);

    on_read(data, read_direction::upstream);
}

void mysql_monitor::dump_data(const bytes_t & data, read_direction direction)
{
    static constexpr size_t half_row_size = std::size_t{ 8 };
    static constexpr size_t row_size = half_row_size * 2;

    std::string out;
    out.reserve((data.size() / row_size + 10) * 80);

    switch (direction)
    {
        case read_direction::downstream:
            out += "Direction: downstream (client -> server)\n";
            break;
        case read_direction::upstream:
            out += "Direction: upstream (server -> client)\n";
            break;
    }

    out += "Packet size: " + std::to_string(data.size()) + "\n\n";

    uint16_t counter = 0;

    size_t i = 0;
    while (i < data.size())
    {
        auto v_size = std::min(i + row_size, data.size());
        std::vector<byte_t> row{ data.cbegin() + i, data.cbegin() + v_size };

        out += to_hex_string(counter) + "  ";

        for (size_t row_i = 0; row_i < row_size; ++row_i)
        {
            if (row_i < row.size())
            {
                out += to_hex_string(row[row_i]) + std::string(" ");
            }
            else
            {
                out += "   ";
            }

            if (row_i == half_row_size - 1)
            {
                out += " ";
            }
        }

        out += ' ';

        auto to_printable = [](const byte_t b) -> char
        {
            if (b > 32 && b < 127)
            {
                return b;
            }
            else
            {
                return '.';
            }
        };

        for (size_t row_i = 0; row_i < row.size(); ++row_i)
        {
            out += to_printable(row[row_i]);

            if (row_i == half_row_size - 1)
            {
                out += ' ';
            }
        }

        out += '\n';

        i += row_size;
        counter += row_size;
    }

    strand_.post([out, this]()
    {
        std::lock_guard<std::mutex> lock_cout(cout_mutex_);
        std::cout << out << "\n---\n" << std::endl;
    });
}

void mysql_monitor::on_read(bytes_t & data, read_direction direction)
{
    dump_data(data, direction);
}

}
