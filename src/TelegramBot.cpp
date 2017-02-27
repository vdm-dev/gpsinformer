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

#include "TelegramBot.h"
#include "TelegramBotHandler.h"
#include "HttpParser.h"



TelegramBot::TelegramBot(asio::io_service& ioService, TelegramBotHandler* handler)
    : _client(ioService)
    , _handler(handler)
    , _lastUpdateId(0)
    , _enabled(false)
{
    _client.setEventHandler(this);
}


TelegramBot::~TelegramBot()
{
}

void TelegramBot::start()
{
    _enabled = true;

    handleHttpClientIdle();
}

void TelegramBot::stop()
{
    _enabled = false;
}

void TelegramBot::makeRequest(const std::string& method, const std::vector<HttpReqArg>& arguments, unsigned int tag, bool longPoll)
{
    if (!_enabled || _apiUrl.empty() || _token.empty() || method.empty())
        return;

    Url url(_apiUrl + _token + "/" + method);

    std::string data = HttpParser::generateRequest(url, arguments, false);

    HttpRequest request(url, data, longPoll, tag);

    _client.sendRequest(request);
}

void TelegramBot::requestUpdates(int32_t limit, int32_t timeout)
{
    std::vector<HttpReqArg> arguments;

    if (_lastUpdateId)
        arguments.push_back(HttpReqArg("offset", _lastUpdateId));

    limit = std::max(1, std::min(100, limit));

    arguments.push_back(HttpReqArg("limit", limit));

    if (timeout)
        arguments.push_back(HttpReqArg("timeout", timeout));

    makeRequest("getUpdates", arguments, TAG_GET_UPDATES, true);
}

void TelegramBot::handleUpdate(const TgBot::Update::Ptr update)
{
    if (!_handler)
        return;

    if (update->inlineQuery != nullptr)
        _handler->handleInlineQuery(update->inlineQuery);

    if (update->chosenInlineResult != nullptr)
        _handler->handleChosenInlineResult(update->chosenInlineResult);

    if (update->callbackQuery != nullptr)
        _handler->handleCallbackQuery(update->callbackQuery);

    if (update->message != nullptr)
        _handler->handleMessage(update->message);
}

void TelegramBot::handleHttpClientError(const system::error_code& error)
{

}

void TelegramBot::handleHttpClientIdle()
{
    if (!_enabled)
        return;

    requestUpdates(100, 60);
}

void TelegramBot::handleHttpClientResponse(const HttpRequest& request, const std::string& response)
{
    using namespace TgBot;

    if (!_enabled)
        return;

    std::string serverResponse = HttpParser::parseResponse(response);

    if (serverResponse.find("<html>") != serverResponse.npos)
        return;

    TgTypeParser& parser = TgTypeParser::getInstance();

    property_tree::ptree json = parser.parseJson(serverResponse);

    if (!json.get("ok", false))
    {
        BOOST_LOG_TRIVIAL(error) << "Telegram error: " << json.get("description", "");
        return;
    }

    json = json.get_child("result");

    switch (request.getTag())
    {
    case TAG_GET_UPDATES:
        std::vector<Update::Ptr> updates = parser.parseJsonAndGetArray<Update>(&TgTypeParser::parseJsonAndGetUpdate, json);
        for (Update::Ptr& item : updates)
        {
            if (item->updateId >= _lastUpdateId)
                _lastUpdateId = item->updateId + 1;

            handleUpdate(item);
        }
        break;
    }
}
