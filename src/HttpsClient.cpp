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

#include "HttpsClient.h"
#include "HttpClientHandler.h"


HttpsClient::HttpsClient(asio::io_service& ioService, HttpClientHandler* handler)
    : _client(ioService)
    , _waiting(false)
    , _handler(handler)
{
    _client.setEventHandler(this);
}

HttpsClient::~HttpsClient()
{

}

void HttpsClient::sendRequest(const HttpRequest& request)
{
    _requestQueue.push_back(request);

    pushQueue();
}

void HttpsClient::pushQueue()
{
    if (_waiting)
        return;

    if (_client.isConnecting() || _client.isConnected())
    {
        if (_requestQueue.front().isLongPoll() && !_response.size())
        {
            if (_client.isConnecting())
                _waiting = true;

            stop(false);
        }

        return;
    }

    if (_requestQueue.empty())
    {
        if (_handler)
            _handler->handleHttpClientIdle();
    }
    else
    {
        Url url = _requestQueue.front().getUrl();

        _client.connect(url.host, url.protocol);
    }
}

void HttpsClient::stop(bool clear)
{
    HttpClientHandler* handler = _handler;

    _handler = 0; // Turn off events
    _client.disconnect(true);
    _handler = handler;

    if (clear)
    {
        _requestQueue.clear();
        _waiting = false;
    }
}

void HttpsClient::handleTcpClientConnect(SslClient* client)
{
    _response.clear();

    if (!_requestQueue.empty())
        _client.send(_requestQueue.front().getData());
}

void HttpsClient::handleTcpClientDisconnect(SslClient* client, TcpClientHandler::Reason reason)
{
    if (_handler)
        _handler->handleHttpClientResponse(_requestQueue.front(), _response);

    _requestQueue.pop_front();
    _response.clear();

    if (reason != TcpClientHandler::ClosedByUser)
    {
        _waiting = false;
        pushQueue();
    }
}

void HttpsClient::handleTcpClientError(SslClient* client, const system::error_code& error)
{
    if (_waiting)
    {
        _waiting = false;
        pushQueue();
    }
    else if (_handler)
    {
        _handler->handleHttpClientError(error);
    }
}

void HttpsClient::handleTcpClientReceivedData(SslClient* client, const std::string& data)
{
    _response += data;
}
