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


#ifndef HttpsClient_INCLUDED
#define HttpsClient_INCLUDED


#include "HttpRequest.h"
#include "SslClient.h"
#include "TcpClientHandler.h"


class HttpClientHandler;


class HttpsClient : public TcpClientHandler<SslClient>
{
public:
    HttpsClient(asio::io_service& ioService, HttpClientHandler* handler = 0);
    ~HttpsClient();

    void sendRequest(const HttpRequest& request);
    void pushQueue();

    void stop();

    void setEventHandler(HttpClientHandler* handler);

private:
    void handleTcpClientConnect(SslClient* client);
    void handleTcpClientDisconnect(SslClient* client, TcpClientHandler::Reason reason);
    void handleTcpClientError(SslClient* client, const system::error_code& error);
    void handleTcpClientReceivedData(SslClient* client, const std::string& data);

    SslClient _client;

    std::deque<HttpRequest> _requestQueue;

    std::string _response;

    HttpClientHandler* _handler;
};


inline void HttpsClient::setEventHandler(HttpClientHandler* handler)
{
    _handler = handler;
}


#endif // HttpsClient_INCLUDED
