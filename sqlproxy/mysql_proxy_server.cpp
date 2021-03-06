﻿#include "stdafx.h"

#include "mysql_proxy_server.h"

#include <iostream>
#include "mysql_monitor.h"

namespace sql_proxy
{

mysql_proxy_server::mysql_proxy_server(boost::asio::io_service& io_service, 
                                       const proxy_server_config_t & config)
    :
    io_service_(io_service),
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(config.local_ip_addr), config.local_port)),
    upstream_endpoint_(boost::asio::ip::address::from_string(config.upstream_ip_addr), config.upstream_port)
{
}

bool mysql_proxy_server::accept_connections()
{
    try
    {
        session_ = std::make_shared<mysql_proxy_session>(io_service_, std::make_shared<mysql_monitor>(io_service_));
        acceptor_.async_accept(
            session_->downstream_socket(),
            [this](const boost::system::error_code error_code)
        {
            handle_accept(error_code);
        });
    }
    catch (const std::exception & ex)
    {
        std::cerr << "Proxy Server exception: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

void mysql_proxy_server::handle_accept(const boost::system::error_code error)
{
    if (!error)
    {
        session_->start(upstream_endpoint_);
        if (!accept_connections())
        {
            std::cerr << "Failure during call to accept." << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: " << error.message() << std::endl;
    }
}

}
