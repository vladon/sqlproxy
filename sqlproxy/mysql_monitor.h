#pragma once

#include "common.h"
#include "mysql_parser.h"

namespace sql_proxy
{

class mysql_monitor : public IProvider
{
public:
    explicit mysql_monitor(boost::asio::io_service & io_service);

    void on_connect() const override;
    void on_downstream_read(bytes_t data) override;
    void on_upstream_read(bytes_t data) override;

private:
    void dump_data(const bytes_t & data, packet_origin direction);

    boost::asio::io_service & io_service_;
    boost::asio::io_service::strand strand_;

    void on_read(const bytes_t & data, packet_origin direction);

    std::mutex downstream_read_mutex_;
    std::mutex upstream_read_mutex_;

    mutable std::mutex cout_mutex_;
    mutable std::mutex dump_mutex_;

    mysql_parser mysql_parser_;
};

}
