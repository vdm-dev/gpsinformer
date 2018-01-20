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
    : _resolver(ioService)
    , _socket(ioService)
    , _readBuffer(32768)
    , _handler(handler)
    , _status(Disconnected)
    , _ioCount(0)
{
}

TcpClient::~TcpClient()
{
}

void TcpClient::connect(const std::string& server, unsigned short port)
{
    connect(server, lexical_cast<std::string>(port));
}

void TcpClient::connect(const std::string& server, const std::string& protocol)
{
    if (_status != Disconnected)
        return;

    _status = Resolve;

    asio::ip::tcp::resolver::query query(server, protocol);

    _ioCount++;
    _resolver.async_resolve(query,
        boost::bind(&TcpClient::handleResolve, this, asio::placeholders::error, asio::placeholders::iterator));
}

void TcpClient::disconnect()
{
    switch (_status)
    {
    case Disconnected:
    case Disconnecting:
        break;
    case Resolve:
        _status = Disconnecting;
        _resolver.cancel();
        break;
    default:
        _status = Disconnecting;
        system::error_code error;
        _socket.close(error);
        break;
    }
}

void TcpClient::send(const unsigned char* buffer, size_t length)
{
    send(std::string((const char*) buffer, length));
}

void TcpClient::send(const std::string& data)
{
    if (_status != Connected)
        return;

    bool inProgress = !_writeBuffer.empty();

    _writeBuffer.push_back(data);

    if (!inProgress)
    {
        _ioCount++;
        asio::async_write(_socket, asio::buffer(_writeBuffer.front().data(), _writeBuffer.front().length()),
            boost::bind(&TcpClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}

bool TcpClient::handleAnything(Status handleStatus, const system::error_code& error)
{
    _ioCount--;

    if (_status == Disconnected)
        return false;

    if (_status == Disconnecting || error)
    {
        system::error_code errorCode;
        _socket.close(errorCode);

        // Waiting for all IO operations to complete
        if (_ioCount > 0)
            return false;

        Status status = _status;

        _writeBuffer.clear();
        _status = Disconnected;
        _ioCount = 0;

        if (!_handler)
            return false;

        if (handleStatus == Connected)
        {
            _handler->handleTcpClientDisconnect(this, 
                status == Disconnecting ? TcpClientHandler<TcpClient>::ClosedByUser : TcpClientHandler<TcpClient>::ClosedByPeer);
        }
        else
        {
            _handler->handleTcpClientError(this, 
                status == Disconnecting ? asio::error::make_error_code(asio::error::basic_errors::operation_aborted) : error);
        }

        return false;
    }

    return true;
}

void TcpClient::handleResolve(const system::error_code& error, asio::ip::tcp::resolver::iterator endpoint)
{
    if (!handleAnything(Resolve, error))
        return;

    _status = Connecting;

    _ioCount++;
    asio::async_connect(_socket, endpoint, boost::bind(&TcpClient::handleConnect, this, asio::placeholders::error));
}

void TcpClient::handleConnect(const system::error_code& error)
{
    if (!handleAnything(Connecting, error))
        return;

    _status = Connected;

    if (_handler)
        _handler->handleTcpClientConnect(this);

    _ioCount++;
    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TcpClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TcpClient::handleRead(size_t size, const system::error_code& error)
{
    system::error_code errorCode = error;

    if (!errorCode && size == 0)
        errorCode = asio::error::make_error_code(asio::error::misc_errors::eof);

    if (!handleAnything(Connected, errorCode))
        return;

    if (_handler)
    {
        std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

        _handler->handleTcpClientReceivedData(this, data);
    }

    _ioCount++;
    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&TcpClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void TcpClient::handleWrite(size_t size, const system::error_code& error)
{
    system::error_code errorCode = error;

    if (!errorCode && size == 0)
        errorCode = asio::error::make_error_code(asio::error::misc_errors::eof);

    if (!handleAnything(Connected, errorCode))
        return;

    _writeBuffer.pop_front();

    if (!_writeBuffer.empty())
    {
        _ioCount++;
        asio::async_write(_socket, asio::buffer(_writeBuffer.front().data(), _writeBuffer.front().length()),
            boost::bind(&TcpClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}
