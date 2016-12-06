#include "StdAfx.h"

#include "TCPSession.h"
#include "TCPServer.h"
#include "TCPSessionHandler.h"

TCPSession::TCPSession(asio::io_service& ioService, TCPServer& server, TCPSessionHandler* handler)
    : _ioService(ioService)
    , _socket(ioService)
    , _readBuffer(32768)
    , _active(false)
    , _server(server)
    , _handler(handler)
{
}

TCPSession::~TCPSession()
{
    std::cout << "[TCPSession] Destructor" << std::endl;
}

void TCPSession::cleanup()
{
    _server._sessions.erase(shared_from_this());

    system::error_code error;
    _socket.close(error);

    _active = false;
}

void TCPSession::close()
{
    if (_handler)
        _handler->handleTCPSessionDisconnect(shared_from_this(), TCPSessionHandler::ClosedByServer);

    cleanup();
}

void TCPSession::disconnect()
{
    if (_handler)
        _handler->handleTCPSessionDisconnect(shared_from_this(), TCPSessionHandler::ClosedByUser);

    cleanup();
}

void TCPSession::send(const std::string& data)
{
    if (!_socket.is_open())
        return;

    bool inProgress = !_writeQueue.empty();

    _writeQueue.push_back(data);

    if (!inProgress)
        write();
}

void TCPSession::start()
{
    _active = true;

    _server._sessions.insert(shared_from_this());

    if (_handler)
        _handler->handleTCPSessionConnect(shared_from_this());

    read();
}

void TCPSession::read()
{
    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TCPSession::handleRead, shared_from_this(), asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TCPSession::write()
{
    asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
        boost::bind(&TCPSession::handleWrite, shared_from_this(), asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TCPSession::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        if (_handler)
            _handler->handleTCPSessionDisconnect(shared_from_this(), TCPSessionHandler::ClosedByPeer);

        cleanup();
        return;
    }

    std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

    if (_handler)
        _handler->handleTCPSessionReceivedData(shared_from_this(), data);

    read();
}

void TCPSession::handleWrite(size_t size, const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTCPSessionDisconnect(shared_from_this(), TCPSessionHandler::ClosedByPeer);

        cleanup();
        return;
    }

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
        write();
}
