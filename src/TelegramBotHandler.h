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


#ifndef TelegramBotHandler_INCLUDED
#define TelegramBotHandler_INCLUDED


#include <tgbot/TgTypeParser.h>

#include "HttpsClient.h"


class TelegramBotHandler
{
public:
    virtual ~TelegramBotHandler() { }

    virtual void handleInlineQuery(const TgBot::InlineQuery::Ptr& inlineQuery) { }
    virtual void handleChosenInlineResult(const TgBot::ChosenInlineResult::Ptr& chosenInlineResult) { }
    virtual void handleCallbackQuery(const TgBot::CallbackQuery::Ptr& callbackQuery) { }
    virtual void handleMessage(const TgBot::Message::Ptr& message) { }
    virtual void handleOwnMessage(const TgBot::Message::Ptr& message) { }
    virtual void handleGetMe(const TgBot::User::Ptr& user) { }
};


#endif // TelegramBotHandler_INCLUDED