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

    // Alert
    chatCommands.push_back(ChatCommand("/arm", "set the system in armed, paranoid or insane mode", ChatCommand::Owner, &Application::handleArmCommand));
    chatCommands.push_back(ChatCommand("/disarm", "disable armed, paranoid or insane mode", ChatCommand::Owner, &Application::handleDisarmCommand));
}

void Application::handleChatCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr& originalMessage)
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

void Application::handleStartCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
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

void Application::handleHelpCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
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

void Application::handlePasswordCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
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
            unsigned int oldAccess = user.access;

            user.access = access;

            if (dbUpdateUser(user))
            {
                output = "Congratulations!\nYour access level has been increased from *";
                output += ChatCommand::getAccessString(oldAccess);
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

void Application::handleGetCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    std::string filter;

    if (command.size() > 1)
        filter = command[1];

    _telegram.sendMessage(originalMessage->from->id, Utilities::treeToString(_settings, filter));
}

void Application::handleSetCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    std::string answerString;

    if (command.size() > 2)
    {
        std::string key = command[1];
        std::string newValue = command[2];

        optional<property_tree::ptree&> option = _settings.get_child_optional(key);

        if (option)
        {
            property_tree::ptree& child = option.get();

            if (child.empty())
            {
                std::string value = child.get_value("");

                child.put_value(newValue);

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

void Application::handleLoadCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    std::string answerString = loadConfiguration() ? 
        "The configuration has been successfully reloaded" : "The configuration reload failed";

    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleSaveCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    std::string answerString = saveConfiguration() ? 
        "The configuration has been successfully saved" : "The configuration save failed";

    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleWhereCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
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

    sendGpsStatus(data[0], originalMessage->from->id);
}

void Application::handleStatusCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    std::vector<GpsMessage> data;

    if (!dbGetGpsData(data, 1, false))
    {
        _telegram.sendMessage(originalMessage->from->id, "I have troubles with database connection. Please try again later.");
        return;
    }

    if (data.empty())
    {
        _telegram.sendMessage(originalMessage->from->id, "Unfortunately there's no data received from tracker.");
        return;
    }

    sendGpsStatus(data[0], originalMessage->from->id);
}

void Application::handleArmCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    unsigned int status = UserSettings::Alert;

    int64_t userId = originalMessage->from->id;

    if (command.size() > 1)
    {
        if (iequals(command[1], "paranoid"))
        {
            status = UserSettings::Paranoid;
        }
        else if (iequals(command[1], "insane"))
        {
            status = UserSettings::Insane;
        }

        if (status != UserSettings::Paranoid && status != UserSettings::Insane)
        {
            _telegram.sendMessage(userId, "Usage: /arm [paranoid | insane]");
            return;
        }
    }

    bool found = false;

    for (size_t i = 0; i < _userSettings.size(); ++i)
    {
        if (_userSettings[i].id == userId)
        {
            _userSettings[i].status = status;

            found = true;
            break;
        }
    }

    if (!found)
    {
        UserSettings settingsRow;

        settingsRow.id = userId;
        settingsRow.status = status;

        _userSettings.push_back(settingsRow);
    }

    switch (status)
    {
    case UserSettings::Alert:
        _telegram.sendMessage(userId, "The system is in *armed* mode now.", TelegramBot::Markdown);
        break;
    case UserSettings::Paranoid:
        _telegram.sendMessage(userId, "The system is in *paranoid* mode now.", TelegramBot::Markdown);
        break;
    case UserSettings::Insane:
        _telegram.sendMessage(userId, "The system is in *insane* mode now.", TelegramBot::Markdown);
        break;
    }

    if (!dbSaveSettings())
    {
        _telegram.sendMessage(userId, "I have troubles with database connection.");
        return;
    }
}

void Application::handleDisarmCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage)
{
    bool found = false;

    int64_t userId = originalMessage->from->id;

    for (size_t i = 0; i < _userSettings.size(); ++i)
    {
        if (_userSettings[i].id == userId)
        {
            _userSettings[i].status = UserSettings::Default;

            found = true;
            break;
        }
    }

    _telegram.sendMessage(userId, "The system is in *disarm* mode now.", TelegramBot::Markdown);

    if (found)
    {
        if (!dbSaveSettings())
        {
            _telegram.sendMessage(userId, "I have troubles with database connection.");
            return;
        }
    }
}
