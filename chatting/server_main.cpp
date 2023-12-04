#include "server.hpp"

namespace asio = boost::asio;

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