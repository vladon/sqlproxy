#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include "common.h"

namespace sql_proxy
{

class basic_proxy_session : public std::enable_shared_from_this<basic_proxy_session>
{
public:
    using socket_t = boost::asio::ip::tcp::socket;

    basic_proxy_session(boost::asio::io_service & io_service, std::shared_ptr<IProvider> provider);
    virtual ~basic_proxy_session() = default;
    
    socket_t & downstream_socket();
    socket_t & upstream_socket();

    void start(const boost::asio::ip::tcp::endpoint & upstream_endpoint);
    void handle_upstream_connect(const boost::system::error_code error_code);

private:
    void handle_downstream_write(const boost::system::error_code error_code,
                                const size_t bytes_transferred);
    void handle_downstream_read(const boost::system::error_code error_code,
                                const size_t bytes_transferred);

    void handle_upstream_write(const boost::system::error_code error_code,
                                const size_t bytes_transferred);
    void handle_upstream_read(const boost::system::error_code error_code,
                              const size_t bytes_transferred);
    void close();

    boost::asio::io_service & io_service_;
    socket_t downstream_socket_;
    socket_t upstream_socket_;

    std::mutex mutex_;

    std::array<uint8_t, max_data_length> downstream_data_;
    std::array<uint8_t, max_data_length> upstream_data_;

    std::shared_ptr<IProvider> provider_;
};

}
