#include "proxy_server.h"

#include <csignal>
#include <iostream>

#include "proxy_session.h"

using boost::asio::ip::tcp;
using boost::asio::ip::address;

proxy_server::proxy_server(const proxy_server_config_t & proxy_server_config)
    :
    io_service_(),
    acceptor_(io_service_),
    socket_(io_service_),
    signals_(io_service_)
{
    signals_.add(SIGABRT);
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.add(SIGBREAK);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif
    do_await_stop();

    tcp::resolver resolver{ io_service_ };
    tcp::endpoint endpoint = *resolver.resolve({ proxy_server_config.bind_host, std::to_string(proxy_server_config.bind_port) });
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
}

void proxy_server::run()
{
    io_service_.run();
}

void proxy_server::do_accept()
{
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec)
    {
        if (!ec)
        {
            std::make_shared<proxy_session>(std::move(socket_))->start();
        }

        do_accept();
    });
}

void proxy_server::do_await_stop()
{
    signals_.async_wait([this](boost::system::error_code ec, int signo)
    {
        if (ec)
        {
            std::cout << "Boost ec: " << ec.message() << std::endl;
        }
        std::cout << "Signal: " << signo << std::endl;
        acceptor_.close();
        io_service_.stop();
    });
}
