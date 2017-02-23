#include "StdAfx.h"

#include "SslClient.h"
#include "TcpClientHandler.h"


SslClient::SslClient(asio::io_service& ioService, TcpClientHandler<SslClient>* handler)
    : _ioService(ioService)
    , _resolver(ioService)
    , _context(asio::ssl::context_base::sslv23)
    , _socket(ioService, _context)
    , _readBuffer(32768)
    , _handler(handler)
{
}

SslClient::~SslClient()
{
}

void SslClient::connect(const std::string& server, unsigned short port)
{
    asio::ip::tcp::resolver::query query(server, lexical_cast<std::string>(port));

    _resolver.async_resolve(query,
        boost::bind(&SslClient::handleResolve, this, asio::placeholders::error, asio::placeholders::iterator));
}

void SslClient::disconnect(bool byUser)
{
    if (_handler)
        _handler->handleTcpClientDisconnect(this, 
            byUser ? TcpClientHandler<SslClient>::ClosedByUser : TcpClientHandler<SslClient>::ClosedByPeer);

    cleanup();
}

void SslClient::cleanup()
{
    system::error_code error;
    _socket.lowest_layer().close(error);
}

void SslClient::send(const std::string& data)
{
    if (!_socket.lowest_layer().is_open())
        return;

    bool inProgress = !_writeQueue.empty();

    _writeQueue.push_back(data);

    if (!inProgress)
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&SslClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}

void SslClient::handleConnect(const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTcpClientError(this, error);

        cleanup();
        return;
    }

    system::error_code errorCode;

    _socket.set_verify_mode(asio::ssl::verify_none, errorCode);
    //_socket.set_verify_callback

    _socket.async_handshake(boost::asio::ssl::stream_base::client, 
        boost::bind(&SslClient::handleHandshake, this, boost::asio::placeholders::error));
}

void SslClient::handleHandshake(const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTcpClientError(this, error);

        cleanup();
        return;
    }

    if (_handler)
        _handler->handleTcpClientConnect(this);

    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&SslClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void SslClient::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        if (_handler)
            _handler->handleTcpClientDisconnect(this, TcpClientHandler<SslClient>::ClosedByPeer);

        cleanup();
        return;
    }

    std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

    if (_handler)
        _handler->handleTcpClientReceivedData(this, data);


    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&SslClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void SslClient::handleResolve(const system::error_code& error, asio::ip::tcp::resolver::iterator endpoint)
{
    if (error)
    {
        if (_handler)
            _handler->handleTcpClientError(this, error);

        return;
    }

    asio::async_connect(_socket.lowest_layer(), endpoint, boost::bind(&SslClient::handleConnect, this, asio::placeholders::error));
}

void SslClient::handleWrite(size_t size, const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTcpClientDisconnect(this, TcpClientHandler<SslClient>::ClosedByPeer);

        cleanup();
        return;
    }

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&SslClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}
