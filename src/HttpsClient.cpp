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
    : _timer(ioService)
    , _client(ioService)
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
    system::error_code errorCode;

    _timer.expires_from_now(posix_time::milliseconds(1), errorCode);

    if (!errorCode)
        _timer.async_wait(bind(&HttpsClient::handleTimerEvent, this));
}

void HttpsClient::stop(bool clear)
{
    HttpClientHandler* handler = _handler;

    _handler = 0; // Turn off events
    _client.disconnect();
    _handler = handler;

    if (clear)
    {
        _requestQueue.clear();
        _response.clear();
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
    if (_handler && reason != TcpClientHandler::ClosedByUser)
        _handler->handleHttpClientResponse(_requestQueue.front(), _response);

    _requestQueue.pop_front();
    _response.clear();

    pushQueue();
}

void HttpsClient::handleTcpClientError(SslClient* client, const system::error_code& error)
{
    _response.clear();

    if (error == asio::error::operation_aborted)
    {
        _requestQueue.pop_front();
        pushQueue();
        return;
    }

    if (_handler)
        _handler->handleHttpClientError(error);
}

void HttpsClient::handleTcpClientReceivedData(SslClient* client, const std::string& data)
{
    _response += data;
}

void HttpsClient::handleTimerEvent()
{
    if (_client.isDisconnecting())
        return;

    if (_client.isConnecting() || _client.isConnected())
    {
        if (_requestQueue.front().isLongPoll() && _response.size() == 0)
            _client.disconnect();

        // Waiting for current request to finish
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
