//
//  Copyright (c) 2017 Dmitry Lavygin (vdm.inbox@gmail.com)
//  Copyright (c) 2015 Oleg Morozenkov
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
    enum ParseMode
    {
        None,
        Markdown,
        Html
    };

    TelegramBot(asio::io_service& ioService, TelegramBotHandler* handler = 0);
    virtual ~TelegramBot();

    void start();
    void stop();

    void setEventHandler(TelegramBotHandler* handler);
    void setApiUrl(const std::string& apiUrl);
    void setToken(const std::string& token);
    void setReconnectDelay(int64_t delay);

    void getMe();
    void sendMessage(int64_t chatId, const std::string& text, ParseMode parseMode, bool disableWebPagePreview, bool disableNotification, int32_t replyToMessageId, const TgBot::GenericReply::Ptr replyMarkup);
    void sendLocation(int64_t chatId, float latitude, float longitude, bool disableNotification, int32_t replyToMessageId, const TgBot::GenericReply::Ptr replyMarkup);
    void sendVenue(int64_t chatId, float latitude, float longitude, const std::string& title, const std::string& address, const std::string& foursquareId, bool disableNotification, int32_t replyToMessageId, const TgBot::GenericReply::Ptr replyMarkup);
    void sendContact(int64_t chatId, const std::string& phoneNumber, const std::string& firstName, const std::string& lastName, bool disableNotification, int32_t replyToMessageId, const TgBot::GenericReply::Ptr replyMarkup);


private:
    enum Tags
    {
        TAG_NONE          = 0,
        TAG_GET_UPDATES   = 1,
        TAG_GET_ME        = 2,
        TAG_SEND_MESSAGE  = 3,
        TAG_SEND_LOCATION = 4,
        TAG_SEND_VENUE    = 5,
        TAG_SEND_CONTACT  = 6
    };

    void getUpdates(int32_t limit, int32_t timeout);

    void makeRequest(const std::string& method, const std::vector<HttpReqArg>& arguments, unsigned int tag = 0, bool longPoll = false);

    void handleUpdate(const TgBot::Update::Ptr update);

    void handleHttpClientError(const system::error_code& error);
    void handleHttpClientIdle();
    void handleHttpClientResponse(const HttpRequest& request, const std::string& response);

    void handleTimerEvent(const system::error_code& error);

    asio::deadline_timer _timer;

    HttpsClient _client;

    std::string _apiUrl;
    std::string _token;

    int32_t _lastUpdateId;
    int64_t _timerDelay;

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

inline void TelegramBot::setReconnectDelay(int64_t delay)
{
    _timerDelay = delay;
}


#endif // TelegramBot_INCLUDED