#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>

// 엔드포인트에 쓰일 포트 번호
// const int PORT_NUMBER = 55555;

namespace asio = boost::asio;

// 서버에서 생성할 세션
class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(asio::io_service& io_service,
                 asio::io_service::strand& strand) : 
                 socket_(io_service), strand_(strand)
        {        
        }

        void start()
        {
            // 연결된 세션의 정보를 출력
            asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint();
            asio::ip::address clientAddress = endpoint.address();
            std::cout << "Client connected from " << clientAddress << std::endl;
            std::cout << "Session start" << std::endl;
        }

        asio::ip::tcp::socket& get_socket() { return socket_; }
    
    private:
        // 연결된 소켓의 정보
        asio::ip::tcp::socket socket_;
        // 비동기 작업의 
        asio::io_service::strand& strand_;
};


class Server
{
    public:
        Server(asio::io_service& io_service,
                asio::io_service::strand& strand,
                asio::ip::tcp::endpoint& endpoint)
                : io_service_(io_service), strand_(strand), acceptor_(io_service, endpoint)
        {
            // bind 사용 시 placeholder 네임스페이스 사용 필요
            using namespace boost::placeholders;
            // 
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
        
        asio::io_service& io_service_;
        asio::io_service::strand& strand_;
        asio::ip::tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2) {
            std::cerr << "Usage: server_name <port>\n";
            return 1;
        }
        // 작업이 없어도 수명 관리와 리소스 관리를 간편화 하기 위해 shared_ptr 사용
        std::shared_ptr<asio::io_service> io_service = std::make_shared<asio::io_service>();
        std::shared_ptr<asio::io_service::work> work = std::make_shared<asio::io_service::work>(*io_service);
        // strand는 lock()과 같은 핸들링 작업이 필요 없어 코드 작성이 간편해짐
        std::shared_ptr<asio::io_service::strand> strand = std::make_shared<asio::io_service::strand>(*io_service);
        
        // 엔드포인트 설정
        // boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER);
        // 엔드포인트를 설정하고 매개변수로 ipv4 서버 객체 생성
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::atoi(argv[1]));
        std::shared_ptr<Server> server = std::make_shared<Server>(*io_service, *strand, endpoint);
        //std::shared_ptr<Server> server(new Server(*io_service, *strand, endpoint));

        // 서버 실행
        io_service->run();

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}