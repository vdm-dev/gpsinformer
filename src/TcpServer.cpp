#include "StdAfx.h"

#include "TcpServer.h"
#include "TcpSessionHandler.h"


TcpServer::TcpServer(asio::io_service& ioService, TcpSessionHandler* handler)
    : _ioService(ioService)
    , _resolver(ioService)
    , _acceptor(ioService)
    , _handler(0)
{
}

TcpServer::~TcpServer()
{
    std::cout << "[TcpServer] Destructor" << std::endl;
}

void TcpServer::listen(const std::string& address, unsigned short port)
{
    system::error_code error;

    asio::ip::tcp::resolver::query query(address, lexical_cast<std::string>(port));

    asio::ip::tcp::resolver::iterator endpointIterator = _resolver.resolve(query, error);
    if (error)
    {
        if (_handler)
            _handler->handleTcpServerError(error);

        return;
    }

    _acceptor.open(endpointIterator->endpoint().protocol(), error);
    if (error)
    {
        if (_handler)
            _handler->handleTcpServerError(error);

        return;
    }

    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), error);
    if (error)
    {
        if (_handler)
            _handler->handleTcpServerError(error);

        return;
    }

    _acceptor.bind(endpointIterator->endpoint(), error);
    if (error)
    {
        if (_handler)
            _handler->handleTcpServerError(error);

        return;
    }

    _acceptor.listen(asio::socket_base::max_connections, error);
    if (error)
    {
        if (_handler)
            _handler->handleTcpServerError(error);

        return;
    }

    accept();
}

void TcpServer::close()
{
    std::set<shared_ptr<TcpSession>>::iterator it = _sessions.begin();

    while (it != _sessions.end())
    {
        std::set<shared_ptr<TcpSession>>::iterator current = it++;

        (*current)->close();
    }

    system::error_code error;
    _acceptor.close(error);
}

void TcpServer::accept()
{
    if (!_acceptor.is_open())
        return;

    shared_ptr<TcpSession> session(new TcpSession(_ioService, *this, _handler));

    _acceptor.async_accept(session->socket(),
        boost::bind(&TcpServer::handleAccept, this, session, asio::placeholders::error));
}

void TcpServer::handleAccept(shared_ptr<TcpSession> session, const system::error_code& error)
{
    if (!error)
        session->start();

    accept();
}
