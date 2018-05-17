#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS

#include <JdWebsocket.h>
#include <JdDbg.h>
#include "json.hpp"
#include "server_ws.hpp"

static int modDbgLevel = CJdDbg::LVL_TRACE;

using namespace std;
using nlohmann::json;

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;


WsServer server;
thread   *myThread;

class CJdWsImplement : public CJdWs
{
public:
    CJdWsImplement(int port)
    {
        mPort = port;
    }
    int Stop();
    int Start();
    int RegisterService(CJdWsService *pService, std::string serviceName);
    int UnregisterService(std::string  serviceName);

    int mPort;
};

CJdWs *CJdWs::Create(int nPort)
{
    JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWs::Create port=%d", nPort));
    CJdWs *res = new CJdWsImplement(nPort);
    return res;
}

int CJdWsImplement::Stop()
{
    server.stop();
    myThread->join();
    return 0;
}

int CJdWsImplement::Start()
{
    JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::Start"));
    server.config.port = mPort;

    myThread = new thread([]() {
        // Start WS-server
        server.start();
    });
    return 0;
}

int CJdWsImplement::RegisterService(CJdWsService *pService, std::string name)
{
    auto &service = server.endpoint[name];//["^/stats/?$"];

    JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::RegisterService"));
    service.on_message = [pService](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {

        std::string reply_msg;
        std::string update_msg;

        std::string command = message->string();
        //JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_open request=%s", command.c_str()));
        json j = json::parse(command);

        //JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_message request=%s", command.c_str()));
        //auto message_str = reply_msg;
        auto message_str = pService->ProcessWsRequest(j.value("action", "none"));
        //JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_message respone=%s", message_str.c_str()));

        auto send_stream = make_shared<WsServer::SendStream>();
        *send_stream << message_str;
        // connection->send is an asynchronous function
        connection->send(send_stream, [](const SimpleWeb::error_code &ec) {
            if (ec) {
                JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_message:connection->send: Error"));
                //cout << "Server: Error sending message. " <<
                // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                //     "Error: " << ec << ", error message: " << ec.message() << endl;
            }

        });

        // echo_all.get_connections() can also be used to solely receive connections on this endpoint
        if(update_msg.size() > 0) {
            for(auto &a_connection : server.get_connections()) {
                auto send_stream = make_shared<WsServer::SendStream>();
                auto message_str = update_msg;
                *send_stream << message_str;
                a_connection->send(send_stream);
            }
        }
    };
    service.on_open = [](shared_ptr<WsServer::Connection> connection) {
        //cout << "Server: Opened connection " << connection.get() << endl;
        JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_open"));
    };

    service.on_close = [](shared_ptr<WsServer::Connection> connection, int status, const string & /*reason*/) {
        //cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
        JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_close"));
    };

    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    service.on_error = [](shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
        //cout << "Server: Error in connection " << connection.get() << ". "
        //     << "Error: " << ec << ", error message: " << ec.message() << endl;
        JDBG_LOG(CJdDbg::LVL_TRACE, ("CJdWsImplement::service.on_error"));
    };

    return 0;
}

int CJdWsImplement::UnregisterService(std::string  serviceName)
{
    return 0;
}