#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>

// 엔드포인트에 쓰일 포트 번호
const int PORT_NUMBER = 55555;

// 서버에서 생성할 세션
class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::io_service& io_service,
                 boost::asio::io_service::strand& strand)
                : socket_(io_service), strand_(strand)
        {        
        }

        void start()
        {
            std::cout << "Session start" << std::endl;
        }

        boost::asio::ip::tcp::socket& get_socket() { return socket_; }
    
    private:
        // 연결된 소켓과의 정보
        boost::asio::ip::tcp::socket socket_;
        boost::asio::io_service::strand& strand_;
};


class Server
{
    public:
        Server(boost::asio::io_service& io_service,
                boost::asio::io_service::strand& strand,
                boost::asio::ip::tcp::endpoint& endpoint)
                : io_service_(io_service), strand_(strand), acceptor_(io_service, endpoint)
        {
            using namespace boost::placeholders;
            std::shared_ptr<Session> session(new Session(io_service_, strand_));
            acceptor_.async_accept(session->get_socket(), strand_.wrap(boost::bind(&Server::start_session, this, session, _1)));
        }

    private:
        void start_session(std::shared_ptr<Session> new_session, const boost::system::error_code& ec)
        {
            if (!ec)
            {
                new_session -> start();
            }

        }

        boost::asio::io_service& io_service_;
        boost::asio::io_service::strand& strand_;
        boost::asio::ip::tcp::acceptor acceptor_;
};


int main(int argc, char* argv[])
{
    try
    {
        // 작업이 없어도 지속성을 위해 work로 수명 관리를 위해 shared_ptr 선언
        std::shared_ptr<boost::asio::io_service> io_service = std::make_shared<boost::asio::io_service>();
        std::shared_ptr<boost::asio::io_service::work> work = std::make_shared<boost::asio::io_service::work>(*io_service);
        std::shared_ptr<boost::asio::io_service::strand> strand = std::make_shared<boost::asio::io_service::strand>(*io_service);
        // 엔드포인트 설정
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER);
        // 비동기 서버로 작업이 없어도 끝나지 않게 설정
        std::shared_ptr<Server> server = std::make_shared<Server>(*io_service, *strand, endpoint);

        
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}