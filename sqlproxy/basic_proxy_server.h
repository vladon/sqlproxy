#pragma once

#include <boost/asio.hpp>

#include "common.h"
#include "basic_proxy_session.h"

namespace sql_proxy
{

class basic_proxy_server
{
public:
    basic_proxy_server(boost::asio::io_service & io_service, const proxy_server_config_t & config);
    virtual ~basic_proxy_server() = default;

    bool accept_connections();

private:
    void handle_accept(const boost::system::error_code error);

    boost::asio::io_service & io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<basic_proxy_session> session_;
    boost::asio::ip::tcp::endpoint upstream_endpoint_;
};

}
