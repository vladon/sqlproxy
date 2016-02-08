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

enum class packet_direction
{
    downstream,
    upstream
};

enum class connection_phase_step
{
    initial_handshake,
    handshake_response
};

class mysql_monitor : public IProvider
{
public:
    explicit mysql_monitor(boost::asio::io_service & io_service);

    void on_connect() const override;
    void on_downstream_read(bytes_t data) override;
    void on_upstream_read(bytes_t data) override;

private:
    void dump_data(const bytes_t & data, packet_direction direction);

    boost::asio::io_service & io_service_;
    boost::asio::io_service::strand strand_;

    void on_read(const bytes_t & data, packet_direction direction);

    bytes_t downstream_packet_;
    read_phase downstream_read_phase_{ read_phase::waiting };
    mysql_packet_header downstream_packet_header_{};
    std::mutex downstream_read_mutex_;

    bytes_t upstream_packet_;
    read_phase upstream_read_phase_{ read_phase::waiting };
    mysql_packet_header upstream_packet_header_{};
    std::mutex upstream_read_mutex_;

    mutable std::mutex cout_mutex_;
    mutable std::mutex dump_mutex_;

    std::atomic_bool initial_handshake_passed_{ false };
};

}
