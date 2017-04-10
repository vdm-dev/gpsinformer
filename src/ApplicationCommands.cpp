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
#include "Utilities.h"
#include "ChatCommand.h"
#include "User.h"


static std::vector<ChatCommand> chatCommands;


void Application::fillCommandList()
{
    chatCommands.clear();

    // Basic
    chatCommands.push_back(ChatCommand("/start", "begin interaction with me", ChatCommand::Default, &Application::handleStartCommand));
    chatCommands.push_back(ChatCommand("/help", "show the list of commands available for you", ChatCommand::Default, &Application::handleHelpCommand));
    chatCommands.push_back(ChatCommand("/password", "verify identity", ChatCommand::Default, &Application::handlePasswordCommand));

    // Settings
    //chatCommands.push_back(ChatCommand("/get", "get current settings tree or branch", ChatCommand::Owner, &Application::handleGetCommand));
    //chatCommands.push_back(ChatCommand("/set", "set new settings", ChatCommand::Owner, &Application::handleSetCommand));
    //chatCommands.push_back(ChatCommand("/load", "reload settings from file", ChatCommand::Owner, &Application::handleLoadCommand));
    //chatCommands.push_back(ChatCommand("/save", "save current settings to file", ChatCommand::Owner, &Application::handleSaveCommand));

    // Tracking
    chatCommands.push_back(ChatCommand("/where", "send last valid location", ChatCommand::User, &Application::handleWhereCommand));
    chatCommands.push_back(ChatCommand("/status", "send last status of tracking device", ChatCommand::User, &Application::handleStatusCommand));

}

void Application::handleChatCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    if (command.empty())
        return;

    User user;

    user.tg = originalMessage->from;

    if (!dbSearchUser(user))
    {
        _telegram.sendMessage(originalMessage->from->id, 
            "Sorry. I have troubles with database connection. Please try again later.");
        return;

    }

    unsigned int access = user.valid ? user.access : ChatCommand::Default;

    for (size_t i = 0; i < chatCommands.size(); ++i)
    {
        if (chatCommands[i].command.empty())
            continue;

        if (iequals(command[0], chatCommands[i].command) && chatCommands[i].handler && (chatCommands[i].access <= access))
            (this->*(chatCommands[i].handler))(command, user, originalMessage);
    }
}

void Application::handleStartCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    std::string output;

    if (user.valid)
    {
        output = "Hey " + user.tg->firstName + "!\nI'm at your service.\nYour access level is *";
        output += ChatCommand::getAccessString(user.access);
        output += "*.";
    }
    else
    {
        output = "Sorry, but I don't know you.\nBefore we proceed further, could you please verify your identity with /password command.";
    }

    _telegram.sendMessage(originalMessage->from->id, output, TelegramBot::Markdown);
    return;
}

void Application::handleHelpCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    std::string output = "You can control me by sending these commands:\n\n";

    unsigned int access = user.valid ? user.access : ChatCommand::Default;

    for (size_t i = 0; i < chatCommands.size(); ++i)
    {
        if (chatCommands[i].command.empty() || (chatCommands[i].access > access))
            continue;

        output += chatCommands[i].command + " - " + chatCommands[i].description + "\n";
    }

    _telegram.sendMessage(originalMessage->from->id, output);
}

void Application::handlePasswordCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    if (command.size() != 2)
    {
        _telegram.sendMessage(originalMessage->from->id, "Usage: /password *code*", TelegramBot::Markdown);
        return;
    }

    std::string ownerCode = _settings.get("code.owner", std::string());
    std::string userCode = _settings.get("code.user", std::string());

    unsigned int access = ChatCommand::Default;

    if (!userCode.empty() && (command[1] == userCode))
        access = ChatCommand::User;

    if (!ownerCode.empty() && (command[1] == ownerCode))
        access = ChatCommand::Owner;


    std::string output = "I have troubles with database connection. Please try again later.";

    if (user.valid)
    {
        if (access > user.access)
        {
            std::string oldAccessString = ChatCommand::getAccessString(user.access);

            user.access = access;

            if (dbUpdateUser(user))
            {
                output = "Congratulations!\nYour access level has been increased from *";
                output += oldAccessString;
                output += "* to *";
                output += ChatCommand::getAccessString(access);
                output += "*.";
            }
        }
        else if (access > ChatCommand::Default)
        {
            output = "Your current access level is *";
            output += ChatCommand::getAccessString(user.access);
            output += "* which is higher or equal than *";
            output += ChatCommand::getAccessString(access);
            output += "*.";
        }
        else
        {
            output = "You've entered wrong code.\nYour current access level is *";
            output += ChatCommand::getAccessString(user.access);
            output += "*.";
        }
    }
    else
    {
        if (access > ChatCommand::Default)
        {
            user.access = access;
            user.tg = originalMessage->from;
            user.valid = true;

            if (dbCreateUser(user))
            {
                output = "Congratulations!\nYou've successfully registered as *";
                output += ChatCommand::getAccessString(access);
                output += "*.";
            }
        }
        else
        {
            output = "Access denied.";
        }
    }


    _telegram.sendMessage(originalMessage->from->id, output, TelegramBot::Markdown);
    return;
}

void Application::handleGetCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    std::string filter;

    if (command.size() > 1)
        filter = command[1];

    _telegram.sendMessage(originalMessage->from->id, Utilities::treeToString(_settings, filter));
}

void Application::handleSetCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    std::string answerString;

    if (command.size() > 2)
    {
        std::string key = command[1];
        std::string newValue = command[2];

        optional<property_tree::ptree&> option = _settings.get_child_optional(key);

        if (option)
        {
            if (option.get().empty())
            {
                std::string value = option.get().get_value("");

                option.get().put_value(newValue);

                answerString = "Value of the key \"" + key + "\" has been changed from \"" + value + "\" to \"" + newValue + "\"";
            }
            else
            {
                answerString = "The key \"" + key + "\" can't hold a value because of subkeys";
            }
        }
        else if ((command.size() > 3) && (command[3] == "!"))
        {
            _settings.add(key, newValue);
            answerString = "The key \"" + key + "\" was successfully added with value \"" + newValue + "\"";
        }
        else
        {
            answerString = "The key \"" + key + "\" doesn't exist";
        }
    }
    else
    {
        answerString = "Usage: /set <key> <value> [!]";
    }


    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleLoadCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    std::string answerString = loadConfiguration() ? 
        "The configuration has been successfully reloaded" : "The configuration reload failed";

    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleSaveCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    std::string answerString = saveConfiguration() ? 
        "The configuration has been successfully saved" : "The configuration save failed";

    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleWhereCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    typedef boost::date_time::c_local_adjustor<posix_time::ptime> timeAdjustor;

    std::vector<GpsMessage> data;

    if (!dbGetGpsData(data, 1, true))
    {
        _telegram.sendMessage(originalMessage->from->id, "I have troubles with database connection. Please try again later.");
        return;
    }

    if (data.empty())
    {
        _telegram.sendMessage(originalMessage->from->id, "Unfortunately there's no valid location received from tracker.");
        return;
    }

    const GpsMessage& gpsMessage = data[0];

    _telegram.sendLocation(originalMessage->from->id, gpsMessage.latitude, gpsMessage.longitude);

    std::string output;

    if (!gpsMessage.hostTime.is_special())
        output += "Host Time: *" + posix_time::to_simple_string(timeAdjustor::utc_to_local(gpsMessage.hostTime)) + "*\n";

    if (!gpsMessage.trackerTime.is_special())
        output += "Tracker Time: " + posix_time::to_simple_string(timeAdjustor::utc_to_local(gpsMessage.trackerTime)) + "\n";

    output += "Speed: " + lexical_cast<std::string>(gpsMessage.speed) + "\n";

    if (!gpsMessage.phone.empty())
        output += "Phone: *" + gpsMessage.phone + "*\n";

    output += "Status: " + gpsMessage.keyword + "\n";

    _telegram.sendMessage(originalMessage->from->id, output, TelegramBot::Markdown);
}

void Application::handleStatusCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr originalMessage)
{
    typedef boost::date_time::c_local_adjustor<posix_time::ptime> timeAdjustor;

    std::vector<GpsMessage> data;

    if (!dbGetGpsData(data, 1, true))
    {
        _telegram.sendMessage(originalMessage->from->id, "I have troubles with database connection. Please try again later.");
        return;
    }

    if (data.empty())
    {
        _telegram.sendMessage(originalMessage->from->id, "Unfortunately there's no data received from tracker.");
        return;
    }

    const GpsMessage& gpsMessage = data[0];

    if (gpsMessage.valid)
        _telegram.sendLocation(originalMessage->from->id, gpsMessage.latitude, gpsMessage.longitude);

    std::string output;

    if (!gpsMessage.hostTime.is_special())
        output += "Host Time: *" + posix_time::to_simple_string(timeAdjustor::utc_to_local(gpsMessage.hostTime)) + "*\n";

    if (!gpsMessage.trackerTime.is_special())
        output += "Tracker Time: " + posix_time::to_simple_string(timeAdjustor::utc_to_local(gpsMessage.trackerTime)) + "\n";

    output += "Speed: " + lexical_cast<std::string>(gpsMessage.speed) + "\n";

    if (!gpsMessage.phone.empty())
        output += "Phone: *" + gpsMessage.phone + "*\n";

    output += "Status: " + gpsMessage.keyword + "\n";

    _telegram.sendMessage(originalMessage->from->id, output, TelegramBot::Markdown);
}
