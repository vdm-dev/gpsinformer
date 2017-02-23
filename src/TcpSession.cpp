#include "StdAfx.h"

#include "TcpSession.h"
#include "TcpServer.h"
#include "TcpSessionHandler.h"


TcpSession::TcpSession(asio::io_service& ioService, TcpServer& server, TcpSessionHandler* handler)
    : _ioService(ioService)
    , _socket(ioService)
    , _readBuffer(32768)
    , _active(false)
    , _server(server)
    , _handler(handler)
{
}

TcpSession::~TcpSession()
{
    std::cout << "[TcpSession] Destructor" << std::endl;
}

void TcpSession::cleanup()
{
    _server._sessions.erase(shared_from_this());

    system::error_code error;
    _socket.close(error);

    _active = false;
}

void TcpSession::close()
{
    if (_handler)
        _handler->handleTcpSessionDisconnect(shared_from_this(), TcpSessionHandler::ClosedByServer);

    cleanup();
}

void TcpSession::disconnect()
{
    if (_handler)
        _handler->handleTcpSessionDisconnect(shared_from_this(), TcpSessionHandler::ClosedByUser);

    cleanup();
}

void TcpSession::send(const std::string& data)
{
    if (!_socket.is_open())
        return;

    bool inProgress = !_writeQueue.empty();

    _writeQueue.push_back(data);

    if (!inProgress)
        write();
}

void TcpSession::start()
{
    _active = true;

    _server._sessions.insert(shared_from_this());

    if (_handler)
        _handler->handleTcpSessionConnect(shared_from_this());

    read();
}

void TcpSession::read()
{
    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TcpSession::handleRead, shared_from_this(), asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TcpSession::write()
{
    asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
        boost::bind(&TcpSession::handleWrite, shared_from_this(), asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TcpSession::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        if (_handler)
            _handler->handleTcpSessionDisconnect(shared_from_this(), TcpSessionHandler::ClosedByPeer);

        cleanup();
        return;
    }

    std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

    if (_handler)
        _handler->handleTcpSessionReceivedData(shared_from_this(), data);

    read();
}

void TcpSession::handleWrite(size_t size, const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTcpSessionDisconnect(shared_from_this(), TcpSessionHandler::ClosedByPeer);

        cleanup();
        return;
    }

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
        write();
}
