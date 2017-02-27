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


#ifndef TelegramBot_INCLUDED
#define TelegramBot_INCLUDED


#include <tgbot/TgTypeParser.h>

#include "HttpsClient.h"
#include "HttpClientHandler.h"
#include "HttpReqArg.h"


class TelegramBotHandler;


class TelegramBot : public HttpClientHandler
{
public:
    TelegramBot(asio::io_service& ioService, TelegramBotHandler* handler = 0);
    virtual ~TelegramBot();

    void start();
    void stop();

    void setEventHandler(TelegramBotHandler* handler);
    void setApiUrl(const std::string& apiUrl);
    void setToken(const std::string& token);

private:
    enum Tags
    {
        TAG_NONE        = 0,
        TAG_GET_UPDATES = 1
    };

    void makeRequest(const std::string& method, const std::vector<HttpReqArg>& arguments, unsigned int tag = 0, bool longPoll = false);

    void requestUpdates(int32_t limit, int32_t timeout);

    void handleUpdate(const TgBot::Update::Ptr update);

    void handleHttpClientError(const system::error_code& error);
    void handleHttpClientIdle();
    void handleHttpClientResponse(const HttpRequest& request, const std::string& response);

    HttpsClient _client;

    std::string _apiUrl;
    std::string _token;

    int32_t _lastUpdateId;

    bool _enabled;

    TelegramBotHandler* _handler;
};


inline void TelegramBot::setEventHandler(TelegramBotHandler* handler)
{
    _handler = handler;
}

inline void TelegramBot::setApiUrl(const std::string& apiUrl)
{
    _apiUrl = apiUrl;
}

inline void TelegramBot::setToken(const std::string& token)
{
    _token = token;
}


#endif // TelegramBot_INCLUDED