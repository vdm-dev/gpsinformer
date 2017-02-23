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

#include "TcpClient.h"
#include "TcpClientHandler.h"


TcpClient::TcpClient(asio::io_service& ioService, TcpClientHandler<TcpClient>* handler)
    : _ioService(ioService)
    , _resolver(ioService)
    , _socket(ioService)
    , _readBuffer(32768)
    , _handler(handler)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::connect(const std::string& server, unsigned short port)
{
    asio::ip::tcp::resolver::query query(server, lexical_cast<std::string>(port));

    system::error_code error;

    asio::ip::tcp::resolver::iterator endpoint = _resolver.resolve(query, error);
    if (error)
    {
        if (_handler)
            _handler->handleTcpClientError(this, error);

        return;
    }

    asio::async_connect(_socket, endpoint, boost::bind(&TcpClient::handleConnect, this, asio::placeholders::error));
}

void TcpClient::disconnect(bool byUser)
{
    if (_handler)
        _handler->handleTcpClientDisconnect(this, 
            byUser ? TcpClientHandler<TcpClient>::ClosedByUser : TcpClientHandler<TcpClient>::ClosedByPeer);

    cleanup();
}

void TcpClient::cleanup()
{
    system::error_code error;
    _socket.close(error);
}

void TcpClient::send(const std::string& data)
{
    if (!_socket.is_open())
        return;

    bool inProgress = !_writeQueue.empty();

    _writeQueue.push_back(data);

    if (!inProgress)
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&TcpClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}

void TcpClient::handleConnect(const system::error_code& error)
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
        boost::bind(&TcpClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TcpClient::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        if (_handler)
            _handler->handleTcpClientDisconnect(this, TcpClientHandler<TcpClient>::ClosedByPeer);

        cleanup();
        return;
    }

    std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

    if (_handler)
        _handler->handleTcpClientReceivedData(this, data);


    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TcpClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TcpClient::handleWrite(size_t size, const system::error_code& error)
{
    if (error)
    {
        if (_handler)
            _handler->handleTcpClientDisconnect(this, TcpClientHandler<TcpClient>::ClosedByPeer);

        cleanup();
        return;
    }

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&TcpClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}
