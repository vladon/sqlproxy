#pragma once

#include <boost/asio.hpp>

#include "common.h"
#include "proxy_bridge.h"

namespace sql_proxy
{

class proxy_server
{
public:
    proxy_server(boost::asio::io_service & io_service, const proxy_server_config_t config);

    bool accept_connections();

private:
    void handle_accept(const boost::system::error_code error);

    boost::asio::io_service & io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<proxy_session> session_;
    boost::asio::ip::tcp::endpoint upstream_endpoint_;
};

}
