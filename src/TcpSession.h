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


#ifndef TcpSession_INCLUDED
#define TcpSession_INCLUDED


class TcpServer;
class TcpSessionHandler;


class TcpSession : public enable_shared_from_this<TcpSession>
{
public:
    TcpSession(asio::io_service& ioService, TcpServer& server, TcpSessionHandler* handler = 0);
    ~TcpSession();

    void cleanup();
    void close();
    void disconnect();
    void send(const std::string& data);
    void start();

    asio::ip::tcp::socket& socket();
    asio::ip::tcp::endpoint endpoint() const;

    bool isActive() const;

private:
    void read();
    void write();

    void handleRead(size_t size, const system::error_code& error);
    void handleWrite(size_t size, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::socket _socket;
    asio::ip::tcp::endpoint _endpoint;

    std::vector<char> _readBuffer;
    std::deque<std::string> _writeQueue;

    bool _active;

    TcpServer& _server;

    TcpSessionHandler* _handler;
};


inline asio::ip::tcp::socket& TcpSession::socket()
{
    return _socket;
}

inline asio::ip::tcp::endpoint TcpSession::endpoint() const
{
    return _endpoint;
}

inline bool TcpSession::isActive() const
{
    return _active;
}


#endif // TcpSession_INCLUDED
