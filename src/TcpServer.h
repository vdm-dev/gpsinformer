#ifndef TcpServer_INCLUDED
#define TcpServer_INCLUDED


#include "TcpSession.h"


class TcpServer
{
public:
    TcpServer(asio::io_service& ioService, TcpSessionHandler* handler = 0);
    ~TcpServer();

    void listen(const std::string& address, unsigned short port);
    void close();

    void setEventHandler(TcpSessionHandler* handler);

private:
    void accept();

    void handleAccept(shared_ptr<TcpSession> session, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::acceptor _acceptor;

    std::set<shared_ptr<TcpSession>> _sessions;

    TcpSessionHandler* _handler;

    friend class TcpSession;
};


inline void TcpServer::setEventHandler(TcpSessionHandler* handler)
{
    _handler = handler;
}


#endif // TcpServer_INCLUDED
