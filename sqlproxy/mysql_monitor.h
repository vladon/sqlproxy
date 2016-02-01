#pragma once

#include "common.h"

namespace sql_proxy
{

struct mysql_packet_header
{
    bool filled{ false };
    uint32_t payload_length; // 3 bytes, little-endian
    uint8_t sequence_id;
};

enum class read_phase
{
    started,
    waiting
};

enum class read_direction
{
    downstream,
    upstream
};

class mysql_monitor : public IProvider
{
public:
    explicit mysql_monitor(boost::asio::io_service & io_service);

    void on_connect() const override;
    void on_downstream_read(bytes_t data) override;
    void on_upstream_read(bytes_t data) override;

private:
    boost::asio::io_service & io_service_;

    void on_read(bytes_t & data, read_direction direction);

    bytes_t downstream_packet_;
    read_phase downstream_read_phase_{ read_phase::waiting };
    mysql_packet_header downstream_packet_header_{};

    mutable std::mutex downstream_read_mutex_;


    bytes_t upstream_packet_;
    read_phase upstream_read_phase_{ read_phase::waiting };
    mysql_packet_header upstream_packet_header_{};

    mutable std::mutex upstream_read_mutex_;
    
    mutable std::mutex cout_mutex_;
};

}
