#include <iostream>
#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>

namespace asio = boost::asio;

// 서버에서 생성할 세션
class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(asio::io_service& io_service,
                 asio::io_service::strand& strand,
                 Server& server) : 
                 socket_(io_service), strand_(strand), server_(server)
        {
        }

        void start()
        {
            // 클라이언트가 연결되면 ip 정보를 출력
            asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint();
            asio::ip::address clientAddress = endpoint.address();
            std::cout << "Client connected from " << clientAddress << std::endl;
            std::cout << "Session start" << std::endl;
        }

        asio::ip::tcp::socket& get_socket()
        {
            return socket_;
        }

        void message_receive(const std::string msg)
        {
            using namespace boost::placeholders;
            
            bool busy = !received_msgs.empty();

            if (!busy)
            {
                boost::asio::async_write(socket_,
                                     boost::asio::buffer(received_msgs.front(), received_msgs.front().size()),
                                     strand_.wrap(boost::bind(&Session::writeHandler, shared_from_this(), _1)));
            }
        }
    
    private:
        void readHandler(const boost::system::error_code& ec)
        {
            using namespace boost::placeholders;

            if (!ec)
            {
                received_msgs.pop();

                if (!received_msgs.empty())
                {
                    boost::asio::async_write(socket_,
                                            boost::asio::buffer(received_msgs.front(), received_msgs.front().size()),
                                            strand_.wrap(boost::bind(&Session::writeHandler, shared_from_this(), _1)));
                }
            }
            else
            {
                server_.leave(shared_from_this());
            }

        }
        void writeHandler(const boost::system::error_code& ec)
        {
            using namespace boost::placeholders;

            if (!ec)
            {
                received_msgs.pop();

                if (!received_msgs.empty())
                {
                    boost::asio::async_write(socket_,
                                            boost::asio::buffer(received_msgs.front(), received_msgs.front().size()),
                                            strand_.wrap(boost::bind(&Session::writeHandler, shared_from_this(), _1)));
                }
            }
            else
            {
                server_.leave(shared_from_this());
            }
        }

        // 연결된 소켓의 정보
        asio::ip::tcp::socket socket_;
        asio::io_service::strand& strand_;
        Server& server_;
        std::queue<std::string> received_msgs;
};


class Server
{
    public:
        Server(asio::io_service& io_service,
                asio::io_service::strand& strand,
                asio::ip::tcp::endpoint& endpoint)
                : io_service_(io_service), strand_(strand), acceptor_(io_service, endpoint)
        {
        }

        void broadcast(const std::string msg)
        {
            using namespace boost::placeholders;
            std::for_each(SessionList.begin(), SessionList.end(), boost::bind(&Session::message_receive, _1, msg));
        }

        void enter(std::shared_ptr<Session> session)
        {
            SessionList.push_back(session);
            broadcast("someone entered");
        }

        void leave(std::shared_ptr<Session> session)
        {
            SessionList.erase(std::remove_if(SessionList.begin(), SessionList.end(), session), SessionList.end());
            broadcast("someone leave");
        }

    private:
    
        void start_session(std::shared_ptr<Session> new_session, const boost::system::error_code& ec)
        {
            if (!ec)
            {
                // bind 사용 시 placeholder 네임스페이스 사용 필요
                using namespace boost::placeholders;
                std::shared_ptr<Session> session(new Session(io_service_, strand_, *this));
                acceptor_.async_accept(session->get_socket(), strand_.wrap(boost::bind(&Server::start_session, this, session, _1)));
                new_session -> start();
            }
        }
        
        asio::io_service& io_service_;
        asio::io_service::strand& strand_;
        asio::ip::tcp::acceptor acceptor_;
        std::vector<std::shared_ptr<Session>> SessionList;
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
        

        // 엔드포인트를 설정하고 ipv4 서버 생성
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