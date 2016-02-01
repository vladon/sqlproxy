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
    io_service_(io_service)
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

void mysql_monitor::on_read(bytes_t & data, read_direction direction)
{
    std::lock_guard<std::mutex> cout_lock(cout_mutex_);

    bytes_t * packet{};
    read_phase * phase{};
    mysql_packet_header * header{};
    auto dir_str = "";

    switch (direction)
    {
        case read_direction::downstream:
            packet = &downstream_packet_;
            phase = &downstream_read_phase_;
            header = &downstream_packet_header_;
            dir_str = "downstream";
            break;

        case read_direction::upstream:
            packet = &upstream_packet_;
            phase = &upstream_read_phase_;
            header = &upstream_packet_header_;
            dir_str = "upstream";
            break;
    }

    if (packet && phase && header)
    {
        if (*phase == read_phase::waiting)
        {
            std::cout << "new data (" << dir_str << "), size: " << data.size() << std::endl;

            packet->clear();
            *phase = read_phase::started;
        }

        if (*phase == read_phase::started)
        {
            std::cout << "packet->size() = " << packet->size() << std::endl;
            std::cout << "appending " << data.size() << " bytes\n";
            packet->insert(packet->end(), data.begin(), data.end());
            std::cout << "packet->size() = " << packet->size() << std::endl;

            if (!header->filled)
            {
                *header = parse_mysql_packet_header(*packet);
            }

            if (header->filled)
            {
                {
                    std::cout << "payload_length = " << header->payload_length << std::endl;
                    std::cout << "sequence_id = " << std::to_string(header->sequence_id) << std::endl;
                }

                packet->reserve(mysql_packet_header_size + header->payload_length);
            }

            if (packet->size() >= mysql_packet_header_size + header->payload_length)
            {
                auto data_size = data.size();
                std::cout << "packet received, size: " << data_size << std::endl;

                std::string hexdata;
                for (auto b : *packet)
                {
                    hexdata += to_hex_string(b);
                }

                std::cout << hexdata << "\n---\n" << std::endl;

                *phase = read_phase::waiting;
                header->filled = false;
                header->payload_length = 0;
                header->sequence_id = 0;
                std::fill(packet->begin(), packet->end(), 0);
                packet->resize(0);
            }
        }
    }
}

}
