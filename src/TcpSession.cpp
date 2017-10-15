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

    if (_handler)
    {
        std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

        _handler->handleTcpSessionReceivedData(shared_from_this(), data);
    }

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
