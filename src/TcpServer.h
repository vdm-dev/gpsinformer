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


#ifndef TcpServer_INCLUDED
#define TcpServer_INCLUDED


#include "TcpSession.h"


class TcpServer
{
public:
    TcpServer(asio::io_service& ioService, TcpSessionHandler* handler = 0);
    ~TcpServer();

    void listen(const std::string& address, unsigned short port);
    void close();

    void setEventHandler(TcpSessionHandler* handler);

private:
    void accept();

    void handleAccept(shared_ptr<TcpSession> session, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::acceptor _acceptor;

    std::set<shared_ptr<TcpSession>> _sessions;

    TcpSessionHandler* _handler;

    friend class TcpSession;
};


inline void TcpServer::setEventHandler(TcpSessionHandler* handler)
{
    _handler = handler;
}


#endif // TcpServer_INCLUDED
