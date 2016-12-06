#ifndef TCPServer_INCLUDED
#define TCPServer_INCLUDED

#include "TCPSession.h"

class TCPServer
{
public:
    TCPServer(asio::io_service& ioService, TCPSessionHandler* handler = 0);
    ~TCPServer();

    void listen(const std::string& address, unsigned short port);
    void close();

    void setEventHandler(TCPSessionHandler* handler);

private:
    void accept();

    void handleAccept(shared_ptr<TCPSession> session, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::acceptor _acceptor;

    std::set<shared_ptr<TCPSession>> _sessions;

    TCPSessionHandler* _handler;

    friend class TCPSession;
};

inline void TCPServer::setEventHandler(TCPSessionHandler* handler)
{
    _handler = handler;
}

#endif // TCPServer_INCLUDED
