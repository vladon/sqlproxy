#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

using host_t = std::string;
using port_t = uint16_t;

struct proxy_server_config_t
{
    host_t bind_host;
    port_t bind_port;
    host_t remote_host;
    port_t remote_port;
};

class proxy_server
{
public:
    explicit proxy_server(const proxy_server_config_t & proxy);
    ~proxy_server() = default;

    void run();

private:
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::signal_set signals_;

    void do_accept();
    void do_await_stop();
};
