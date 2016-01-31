#include "stdafx.h"

#include "proxy_bridge.h"

#include <csignal>
#include <iostream>

namespace sql_proxy
{

proxy_session::proxy_session(boost::asio::io_service& io_service)
    :
    io_service_(io_service),
    downstream_socket_(io_service),
    upstream_socket_(io_service)
{
}

proxy_session::socket_t& proxy_session::downstream_socket()
{
    return downstream_socket_;
}

proxy_session::socket_t& proxy_session::upstream_socket()
{
    return upstream_socket_;
}

void proxy_session::start(const boost::asio::ip::tcp::endpoint & upstream_endpoint)
{
    auto self(shared_from_this());
    upstream_socket_.async_connect(
        upstream_endpoint,
        [this, self](const boost::system::error_code error_code)
    {
        shared_from_this()->handle_upstream_connect(error_code);
    });
}

void proxy_session::handle_upstream_connect(const boost::system::error_code error)
{
    if (!error)
    {
        {
            auto self(shared_from_this());
            upstream_socket_.async_read_some(
                boost::asio::buffer(upstream_data_),
                [this, self](const boost::system::error_code error_code,
                             const size_t bytes_transferred)
            {
                shared_from_this()->handle_upstream_read(error_code, bytes_transferred);
            });
        }

        {
            auto self(shared_from_this());
            downstream_socket_.async_read_some(
                boost::asio::buffer(downstream_data_),
                [this, self](const boost::system::error_code error_code,
                       const size_t bytes_transferred)
            {
                shared_from_this()->handle_downstream_read(error_code, bytes_transferred);
            });
        }
    }
    else
    {
        close();
    }
}

void proxy_session::handle_downstream_write(const boost::system::error_code t_error_code, const size_t t_bytes_transferred)
{
    if (!t_error_code)
    {
        auto self(shared_from_this());
        upstream_socket_.async_read_some(
            boost::asio::buffer(upstream_data_),
            [this, self](const boost::system::error_code error_code,
                   const size_t bytes_transferred)
        {
            shared_from_this()->handle_upstream_read(error_code, bytes_transferred);
        });
    }
    else
    {
        close();
    }
}

void proxy_session::handle_downstream_read(const boost::system::error_code t_error_code, const size_t t_bytes_transferred)
{
    if (!t_error_code)
    {
        auto self(shared_from_this());
        boost::asio::async_write(
            upstream_socket_,
            boost::asio::buffer(downstream_data_, t_bytes_transferred),
            [this, self](const boost::system::error_code error_code, const size_t bytes_transferred)
        {
            shared_from_this()->handle_upstream_write(error_code, bytes_transferred);
        });
    }
    else
    {
        close();
    }
}

void proxy_session::handle_upstream_write(const boost::system::error_code t_error_code, const size_t t_bytes_transferred)
{
    if (!t_error_code)
    {
        auto self(shared_from_this());
        downstream_socket_.async_read_some(
            boost::asio::buffer(downstream_data_),
            [this, self](const boost::system::error_code error_code,
                   const size_t bytes_transferred)
        {
            shared_from_this()->handle_downstream_read(error_code, bytes_transferred);
        });
    }
    else
    {
        close();
    }
}

void proxy_session::handle_upstream_read(const boost::system::error_code t_error_code, const size_t t_bytes_transferred)
{
    if (!t_error_code)
    {
        auto self(shared_from_this());
        boost::asio::async_write(
            downstream_socket_,
            boost::asio::buffer(upstream_data_, t_bytes_transferred),
            [this, self](const boost::system::error_code error_code, const size_t bytes_transferred)
        {
            shared_from_this()->handle_downstream_write(error_code, bytes_transferred);
        });
    }
    else
    {
        close();
    }
}

void proxy_session::close()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto close_socket = [](boost::asio::ip::tcp::socket & socket)
    {
        if (socket.is_open())
        {
            boost::system::error_code ec;
            socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            socket.close(ec);
        }
    };

    close_socket(downstream_socket_);
    close_socket(upstream_socket_);
}
} // namespace sql_proxy
