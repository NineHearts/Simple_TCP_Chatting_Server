#include "session.hpp"
#include "server.hpp"

void Session::init()
{
    // 세션에 연결된 클라이언트의 ip 정보를 출력
    asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint();
    asio::ip::address clientAddress = endpoint.address();
    std::cout << "Client connected from " << clientAddress << std::endl;
    std::cout << "Session start" << std::endl;
    server_.enter(shared_from_this());
}

void Session::message_receive(const std::string msg)
{
    bool busy = !received_msgs.empty();
    if (!busy)
    {
        //std::cout << "msg receive" << std::endl;
        boost::asio::async_write(socket_,
                                boost::asio::buffer(received_msgs.front(), received_msgs.front().size()),
                                strand_.wrap(boost::bind(&Session::writeHandler, shared_from_this(), 
                                                boost::placeholders::_1)));
    }
}

void Session::readHandler(const boost::system::error_code& ec)
{
    if (!ec)
    {
        server_.broadcast(received_msgs.front());

        if (!received_msgs.empty())
        {
            boost::asio::async_read(socket_, boost::asio::buffer(received_msgs.front(), received_msgs.front().size()),
                                    strand_.wrap(boost::bind(&Session::readHandler, 
                                                                shared_from_this(), 
                                                                boost::placeholders::_1)));
        }
    }
    else
    {
        server_.leave(shared_from_this());
    }

}
void Session::writeHandler(const boost::system::error_code& ec)
{
    if (!ec)
    {
        write_msgs.pop();

        if (!write_msgs.empty())
        {
            boost::asio::async_write(socket_, boost::asio::buffer(write_msgs.front(), write_msgs.front().size()),
                                    strand_.wrap(boost::bind(&Session::writeHandler, 
                                                                shared_from_this(), 
                                                                boost::placeholders::_1)));
        }
    }
    else
    {
        server_.leave(shared_from_this());
    }
}