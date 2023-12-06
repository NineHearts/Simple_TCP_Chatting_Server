#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

namespace asio = boost::asio;

class Server;

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(asio::io_service& io_service,
                 asio::io_service::strand& strand,
                 Server& server) : 
                 socket_(io_service), strand_(strand), server_(server){};

        void init();

        asio::ip::tcp::socket& get_socket() { return socket_; }
        void message_receive(const std::string msg);
    
    private:
        void readHandler(const boost::system::error_code& ec);
        void writeHandler(const boost::system::error_code& ec);

        asio::ip::tcp::socket socket_;
        asio::io_service::strand& strand_;
        Server& server_;
        std::queue<std::string> received_msgs;
        std::queue<std::string> write_msgs;
};