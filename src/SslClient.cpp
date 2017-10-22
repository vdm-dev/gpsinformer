//
//  Copyright (c) 2017 Dmitry Lavygin (vdm.inbox@gmail.com)
// 
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
// 
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
// 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//


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
    , _connecting(false)
    , _established(false)
{
}

SslClient::~SslClient()
{
}

void SslClient::connect(const std::string& server, unsigned short port)
{
    connect(server, lexical_cast<std::string>(port));
}

void SslClient::connect(const std::string& server, const std::string& protocol)
{
    _connecting = true;
    _established = false;

    asio::ip::tcp::resolver::query query(server, protocol);

    _resolver.async_resolve(query,
        boost::bind(&SslClient::handleResolve, this, asio::placeholders::error, asio::placeholders::iterator));
}

void SslClient::disconnect(bool byUser)
{
    cleanup();

    if (_handler)
        _handler->handleTcpClientDisconnect(this, 
            byUser ? TcpClientHandler<SslClient>::ClosedByUser : TcpClientHandler<SslClient>::ClosedByPeer);
}

void SslClient::cleanup()
{
    _resolver.cancel();

    system::error_code error;
    _socket.lowest_layer().close(error);
    _socket.shutdown(error);

    _connecting = false;
    _established = false;
}

void SslClient::send(const std::string& data)
{
    if (!isConnected())
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
        cleanup();

        if (_handler)
            _handler->handleTcpClientError(this, error);

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
        cleanup();

        if (_handler)
            _handler->handleTcpClientError(this, error);

        return;
    }

    _established = true;

    if (_handler)
        _handler->handleTcpClientConnect(this);

    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&SslClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void SslClient::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        cleanup();

        if (_handler)
            _handler->handleTcpClientDisconnect(this, TcpClientHandler<SslClient>::ClosedByPeer);

        return;
    }

    if (_handler)
    {
        std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

        _handler->handleTcpClientReceivedData(this, data);
    }


    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&SslClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void SslClient::handleResolve(const system::error_code& error, asio::ip::tcp::resolver::iterator endpoint)
{
    // Connection aborted
    if (!_connecting && !error)
    {
        if (_handler)
            _handler->handleTcpClientError(this, asio::error::make_error_code(asio::error::operation_aborted));

        return;
    }

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
        cleanup();

        if (_handler)
            _handler->handleTcpClientDisconnect(this, TcpClientHandler<SslClient>::ClosedByPeer);

        return;
    }

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&SslClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}
