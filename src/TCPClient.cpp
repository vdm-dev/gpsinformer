#include "StdAfx.h"

#include "TCPClient.h"
#include "TCPClientHandler.h"

TCPClient::TCPClient(asio::io_service& ioService, TCPClientHandler* handler)
    : _ioService(ioService)
    , _resolver(ioService)
    , _socket(ioService)
    , _readBuffer(32768)
    , _handler(handler)
{
}

TCPClient::~TCPClient()
{
}

void TCPClient::connect(const std::string& server, unsigned short port)
{
    asio::ip::tcp::resolver::query query(server, lexical_cast<std::string>(port));

    system::error_code error;

    asio::ip::tcp::resolver::iterator endpoint = _resolver.resolve(query, error);
    if (error)
    {
        if (_handler)
            _handler->handleTCPClientError(this, error);

        return;
    }

    asio::async_connect(_socket, endpoint, boost::bind(&TCPClient::handleConnect, this, asio::placeholders::error));
}

void TCPClient::disconnect(bool byUser)
{
    if (_handler)
        _handler->handleTCPClientDisconnect(this, byUser ? TCPClientHandler::ClosedByUser : TCPClientHandler::ClosedByPeer);

    cleanup();
}

void TCPClient::cleanup()
{
    system::error_code error;
    _socket.close(error);
}

void TCPClient::send(const std::string& data)
{
    if (!_socket.is_open())
        return;

    bool inProgress = !_writeQueue.empty();

    _writeQueue.push_back(data);

    if (!inProgress)
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&TCPClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}

void TCPClient::handleConnect(const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTCPClientError(this, error);

        cleanup();
        return;
    }

    if (_handler)
        _handler->handleTCPClientConnect(this);

    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TCPClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TCPClient::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        if (_handler)
            _handler->handleTCPClientDisconnect(this, TCPClientHandler::ClosedByPeer);

        cleanup();
        return;
    }

    std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

    if (_handler)
        _handler->handleTCPClientReceivedData(this, data);


    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TCPClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TCPClient::handleWrite(size_t size, const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTCPClientDisconnect(this, TCPClientHandler::ClosedByPeer);

        cleanup();
        return;
    }

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&TCPClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}
