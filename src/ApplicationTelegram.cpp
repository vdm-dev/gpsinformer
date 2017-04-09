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

#include "Application.h"


void Application::startTelegram()
{
    bool        enabled    = _settings.get("telegram.enabled", false);
    std::string url        = _settings.get("telegram.url", std::string("https://api.telegram.org/bot"));
    std::string token      = _settings.get("telegram.token", std::string());
    int64_t reconnectDelay = _settings.get("telegram.reconnect_delay", 0);

    if (!enabled)
    {
        BOOST_LOG_TRIVIAL(info) << "Telegram Bot is disabled";
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Telegram Bot is enabled";

    _telegram.setApiUrl(url);
    _telegram.setToken(token);
    _telegram.setReconnectDelay(reconnectDelay);

    _telegram.start();

    _telegram.getMe();
}

void Application::handleInlineQuery(const TgBot::InlineQuery::Ptr inlineQuery)
{

}

void Application::handleChosenInlineResult(const TgBot::ChosenInlineResult::Ptr chosenInlineResult)
{

}

void Application::handleCallbackQuery(const TgBot::CallbackQuery::Ptr callbackQuery)
{

}

void Application::handleMessage(const TgBot::Message::Ptr message)
{
    if (!message)
        return;

    std::string body = message->text;

    if (body.empty() || (body[0] != '/'))
        return;

    std::string argument;

    std::vector<std::string> command;

    bool quoted = false;

    for (size_t i = 0; i < body.size(); ++i)
    {
        if (!quoted)
        {
            if ((body[i] == '\r') || (body[i] == '\n'))
            {
                if (argument.size())
                    command.push_back(argument);

                argument.clear();

                handleChatCommand(command, message);

                command.clear();
                continue;
            }
            else if ((body[i] == ' ') || (body[i] == '"'))
            {
                if (argument.size())
                    command.push_back(argument);

                argument.clear();

                quoted = (body[i] == '"');

                continue;
            }
        }
        else if (body[i] == '"')
        {
            command.push_back(argument);
            argument.clear();
            quoted = false;
            continue;
        }

        argument += body[i];
    }

    if (argument.size() || quoted)
        command.push_back(argument);

    handleChatCommand(command, message);
}

void Application::handleOwnMessage(const TgBot::Message::Ptr message)
{

}

void Application::handleGetMe(const TgBot::User::Ptr user)
{

}
