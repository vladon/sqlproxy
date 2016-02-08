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

class mysql_proxy_session : public std::enable_shared_from_this<mysql_proxy_session>
{
public:
    using socket_t = boost::asio::ip::tcp::socket;

    mysql_proxy_session(boost::asio::io_service & io_service, std::shared_ptr<IProvider> provider);
    virtual ~mysql_proxy_session() = default;
    
    socket_t & downstream_socket();
    socket_t & upstream_socket();

    void start(const boost::asio::ip::tcp::endpoint & upstream_endpoint);
    void handle_upstream_connect(const boost::system::error_code error_code);

private:
    using data_t = std::array<uint8_t, max_data_length>;
    using read_handler_t = std::function<void(const boost::system::error_code, const size_t)>;

    void co_read(socket_t & socket, data_t & data, read_handler_t read_handler)
    {
        auto self(shared_from_this());
        boost::asio::spawn(io_service_,
                           [this, self, &socket, &data, read_handler](boost::asio::yield_context yield)
        {
            boost::system::error_code ec;

            boost::asio::async_read(socket,
                                    boost::asio::buffer(data),
                                    boost::asio::transfer_exactly(sizeof(mysql_packet_header)),
                                    yield[ec]);

            if (!ec)
            {
                auto header = reinterpret_cast<mysql_packet_header *>(data.data());
                auto payload_length = header->get_payload_length();
                auto bytes_transferred =
                    sizeof(mysql_packet_header) +
                    boost::asio::async_read(
                        socket,
                        boost::asio::buffer(data.data() + sizeof(mysql_packet_header), payload_length),
                        boost::asio::transfer_exactly(payload_length),
                        yield[ec]);
                read_handler(ec, bytes_transferred);
            }
        });
    }

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

    data_t downstream_data_{};
    data_t upstream_data_{};

    std::shared_ptr<IProvider> provider_;
};

}
