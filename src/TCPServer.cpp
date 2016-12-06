#include "StdAfx.h"

#include "TCPServer.h"
#include "TCPSessionHandler.h"

TCPServer::TCPServer(asio::io_service& ioService, TCPSessionHandler* handler)
    : _ioService(ioService)
    , _resolver(ioService)
    , _acceptor(ioService)
    , _handler(0)
{
}

TCPServer::~TCPServer()
{
    std::cout << "[TCPServer] Destructor" << std::endl;
}

void TCPServer::listen(const std::string& address, unsigned short port)
{
    system::error_code error;

    asio::ip::tcp::resolver::query query(address, lexical_cast<std::string>(port));

    asio::ip::tcp::resolver::iterator endpointIterator = _resolver.resolve(query, error);
    if (error)
    {
        if (_handler)
            _handler->handleTCPServerError(error);

        return;
    }

    _acceptor.open(endpointIterator->endpoint().protocol(), error);
    if (error)
    {
        if (_handler)
            _handler->handleTCPServerError(error);

        return;
    }

    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), error);
    if (error)
    {
        if (_handler)
            _handler->handleTCPServerError(error);

        return;
    }

    _acceptor.bind(endpointIterator->endpoint(), error);
    if (error)
    {
        if (_handler)
            _handler->handleTCPServerError(error);

        return;
    }

    _acceptor.listen(asio::socket_base::max_connections, error);
    if (error)
    {
        if (_handler)
            _handler->handleTCPServerError(error);

        return;
    }

    accept();
}

void TCPServer::close()
{
    std::set<shared_ptr<TCPSession>>::iterator it = _sessions.begin();

    while (it != _sessions.end())
    {
        std::set<shared_ptr<TCPSession>>::iterator current = it++;

        (*current)->close();
    }

    system::error_code error;
    _acceptor.close(error);
}

void TCPServer::accept()
{
    if (!_acceptor.is_open())
        return;

    shared_ptr<TCPSession> session(new TCPSession(_ioService, *this, _handler));

    _acceptor.async_accept(session->socket(),
        boost::bind(&TCPServer::handleAccept, this, session, asio::placeholders::error));
}

void TCPServer::handleAccept(shared_ptr<TCPSession> session, const system::error_code& error)
{
    if (!error)
        session->start();

    accept();
}
