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


#ifndef TcpClient_INCLUDED
#define TcpClient_INCLUDED


template <class T>
class TcpClientHandler;


class TcpClient
{
public:
    TcpClient(asio::io_service& ioService, TcpClientHandler<TcpClient>* handler = 0);
    ~TcpClient();

    void connect(const std::string& server, unsigned short port);
    void disconnect(bool byUser = true);

    void cleanup();

    void send(const std::string& data);

    void setEventHandler(TcpClientHandler<TcpClient>* handler);

    asio::ip::tcp::socket& socket();

private:
    void handleConnect(const system::error_code& error);
    void handleRead(size_t size, const system::error_code& error);
    void handleWrite(size_t size, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::socket _socket;

    std::vector<char> _readBuffer;
    std::deque<std::string> _writeQueue;

    TcpClientHandler<TcpClient>* _handler;
};


inline void TcpClient::setEventHandler(TcpClientHandler<TcpClient>* handler)
{
    _handler = handler;
}

inline asio::ip::tcp::socket& TcpClient::socket()
{
    return _socket;
}


#endif // TcpClient_INCLUDED
