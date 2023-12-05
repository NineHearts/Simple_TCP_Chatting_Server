#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "session.hpp"

class Server
{
    public:
        Server(asio::io_service& io_service,
                asio::io_service::strand& strand,
                asio::ip::tcp::endpoint& endpoint)
                : io_service_(io_service), strand_(strand), acceptor_(io_service, endpoint)
        {
            run();
        }

        void broadcast(const std::string msg)
        {
            std::for_each(SessionList.begin(), SessionList.end(), 
                            boost::bind(&Session::message_receive, 
                            boost::placeholders::_1, msg));
        }

        void enter(std::shared_ptr<Session> session)
        {
            SessionList.push_back(session);
        }

        void leave(std::shared_ptr<Session> session)
        {
            SessionList.erase(std::remove(SessionList.begin(), SessionList.end(), session), SessionList.end());
        }

    private:

        void run()
        {
            std::shared_ptr<Session> session(new Session(io_service_, strand_, *this));
            acceptor_.async_accept(session->get_socket(), strand_.wrap(boost::bind(&Server::start_session, this, session, 
                                                                                    boost::placeholders::_1)));
        }
        void start_session(std::shared_ptr<Session> new_session, const boost::system::error_code& ec)
        {
            if (!ec)
            {
                new_session -> init();
            }
            else
                std::cout << "Session init failed" << std::endl;
            run();
        }
        
        asio::io_service& io_service_;
        asio::io_service::strand& strand_;
        asio::ip::tcp::acceptor acceptor_;
        std::vector<std::shared_ptr<Session>> SessionList;
};
