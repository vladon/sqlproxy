﻿#include "stdafx.h"

#include "mysql_monitor.h"

#include "mysql_parser.h"

namespace sql_proxy
{

constexpr size_t mysql_packet_header_size = 4;

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

    on_read(data, packet_origin::downstream);
}

void mysql_monitor::on_upstream_read(bytes_t data)
{
    std::lock_guard<std::mutex> upstream_read_lock(upstream_read_mutex_);

    on_read(data, packet_origin::upstream);
}

void mysql_monitor::dump_data(const bytes_t & data, packet_origin direction)
{
    std::lock_guard<std::mutex> dump_lock(dump_mutex_);

    strand_.post([this, data, direction]()
    {
        static constexpr size_t half_row_size = std::size_t{ 8 };
        static constexpr size_t row_size = half_row_size * 2;

        std::string out;
        out.reserve((data.size() / row_size + 10) * 80);

        out += mysql_parser_.parse_packet(data, direction);

        out += "\n=== DUMP BEGIN ===\n";
        switch (direction)
        {
            case packet_origin::downstream:
                out += "Direction: downstream (client -> server)\n";
                break;
            case packet_origin::upstream:
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
                if (b >= ' ' && b < 127)
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

        out += "=== DUMP END ===\n\n---\n";

        {
            std::lock_guard<std::mutex> lock_cout(cout_mutex_);
            std::cout << out;
        }
    });
}

void mysql_monitor::on_read(const bytes_t & data, packet_origin direction)
{
    //strand_.post([this](){
    //    std::lock_guard<std::mutex> lock_cout(cout_mutex_);
    //    std::cout << "!!!!!!!!!! on_read" << std::endl;
    //});
    dump_data(data, direction);
}

}
