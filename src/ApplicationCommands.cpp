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

#include <sqlite3.h>


struct ChatCommand
{
    typedef void(Application::*ChatCommandHandler)(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage);

    enum Access
    {
        Default = 0,
        User = 1,
        Owner = 2
    };

    static std::string getAccessString(unsigned int level)
    {
        switch (level)
        {
        case ChatCommand::Owner:
            return "owner";
        case ChatCommand::User:
            return "user";
        default:
            return "unprivileged";
        }
    }

    ChatCommand(const std::string& command, const std::string& description, unsigned int access, ChatCommandHandler handler)
        : command(command)
        , description(description)
        , access(access)
        , handler(handler)
    {
    }

    std::string command;
    std::string description;
    unsigned int access;

    ChatCommandHandler handler;
};


static std::vector<ChatCommand> chatCommands;


bool createUser(sqlite3* database, const TgBot::User::Ptr user, unsigned int access = ChatCommand::Default)
{
    sqlite3_stmt *stmt = 0;

    char* sql = "INSERT INTO users (id, firstname, lastname, nickname, access) VALUES (?, ?, ?, ?, ?)";

    int result = sqlite3_prepare_v2(database, sql, -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 1, user->id);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 2, user->firstName.c_str(), user->firstName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 3, user->lastName.c_str(), user->lastName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 4, user->username.c_str(), user->username.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 5, access);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);
    if ((result != SQLITE_OK) && (result != SQLITE_DONE))
        goto error;

    sqlite3_finalize(stmt);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(database);
    return false;
}

bool updateUser(sqlite3* database, const TgBot::User::Ptr user, unsigned int access = ChatCommand::Default)
{
    sqlite3_stmt *stmt = 0;

    char* sql = "UPDATE users SET firstname = ?, lastname = ?, nickname = ?, access = ? WHERE id = ?";

    int result = sqlite3_prepare_v2(database, sql, -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 1, user->firstName.c_str(), user->firstName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 2, user->lastName.c_str(), user->lastName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 3, user->username.c_str(), user->username.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 4, access);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 5, user->id);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);
    if ((result != SQLITE_OK) && (result != SQLITE_DONE))
        goto error;

    sqlite3_finalize(stmt);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(database);
    return false;
}

bool searchUser(sqlite3* database, const TgBot::User::Ptr user, bool* found = 0, unsigned int* access = 0)
{
    sqlite3_stmt *stmt = 0;

    char* sql = "SELECT * FROM users WHERE id = ? LIMIT 1";

    int result = sqlite3_prepare_v2(database, sql, -1, &stmt, 0);

    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 1, user->id);

    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);

    if (found)
        *found = false;

    bool needUpdate = false;

    unsigned int userAccess = ChatCommand::Default;

    while (result == SQLITE_ROW)
    {
        if (found)
            *found = true;

        int columns = sqlite3_column_count(stmt);

        for (int column = 0; column < columns; ++column)
        {
            std::string columnName = sqlite3_column_name(stmt, column);

            if (columnName == "firstname")
            {
                std::string text = (const char*) sqlite3_column_text(stmt, column);
                needUpdate |= (text != user->firstName);
            }
            else if (columnName == "lastname")
            {
                std::string text = (const char*) sqlite3_column_text(stmt, column);
                needUpdate |= (text != user->lastName);
            }
            else if (columnName == "nickname")
            {
                std::string text = (const char*) sqlite3_column_text(stmt, column);
                needUpdate |= (text != user->username);
            }
            else if (columnName == "access")
            {
                userAccess = sqlite3_column_int(stmt, column);

                if (access)
                    *access = userAccess;
            }
        }

        result = sqlite3_step(stmt);
    }

    if ((result != SQLITE_DONE) && (result != SQLITE_OK))
        goto error;

    result = sqlite3_finalize(stmt);

    if (result != SQLITE_OK)
        goto error;

    if (needUpdate)
        updateUser(database, user, userAccess);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(database);
    return false;
}


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
    chatCommands.push_back(ChatCommand("/load", "reload settings from file", ChatCommand::Owner, &Application::handleLoadCommand));
    chatCommands.push_back(ChatCommand("/save", "save current settings to file", ChatCommand::Owner, &Application::handleSaveCommand));

    // Tracking
    chatCommands.push_back(ChatCommand("/where", "send last valid location", ChatCommand::User, &Application::handleWhereCommand));
    chatCommands.push_back(ChatCommand("/status", "send last status of tracking device", ChatCommand::User, &Application::handleStatusCommand));

}

void Application::handleChatCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    if (command.empty())
        return;

    unsigned int access = ChatCommand::Default;

    searchUser(_database, originalMessage->from, 0, &access);

    for (size_t i = 0; i < chatCommands.size(); ++i)
    {
        if (chatCommands[i].command.empty())
            continue;

        if (iequals(command[0], chatCommands[i].command) && chatCommands[i].handler && (chatCommands[i].access <= access))
            (this->*(chatCommands[i].handler))(command, originalMessage);
    }
}

void Application::handleStartCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    std::string output = "Hi! I have troubles with database connection. Please try again later.";

    bool found = false;

    unsigned int access = 0;

    if (searchUser(_database, originalMessage->from, &found, &access))
    {
        if (found)
        {
            output = "Hey " + originalMessage->from->firstName + "!\nI'm at your service.\nYour access level is *";
            output += ChatCommand::getAccessString(access);
            output += "*.";
        }
        else
        {
            output = "Sorry, but I don't know you.\nBefore we proceed further, could you please verify your identity with /password command.";
        }
    }

    _telegram.sendMessage(originalMessage->from->id, output, TelegramBot::Markdown);
    return;
}

void Application::handleHelpCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    std::string output = "You can control me by sending these commands:\n\n";

    unsigned int access = ChatCommand::Default;

    searchUser(_database, originalMessage->from, 0, &access);

    for (size_t i = 0; i < chatCommands.size(); ++i)
    {
        if (chatCommands[i].command.empty() || (chatCommands[i].access > access))
            continue;

        output += chatCommands[i].command + " - " + chatCommands[i].description + "\n";
    }

    _telegram.sendMessage(originalMessage->from->id, output);
}

void Application::handlePasswordCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    if (command.size() != 2)
    {
        _telegram.sendMessage(originalMessage->from->id, "Usage: /password *code*", TelegramBot::Markdown);
        return;
    }

    std::string ownerCode = _settings.get("code.owner", std::string());
    std::string userCode = _settings.get("code.user", std::string());

    unsigned int access = 0;

    if (!userCode.empty() && (command[1] == userCode))
        access = ChatCommand::User;

    if (!ownerCode.empty() && (command[1] == ownerCode))
        access = ChatCommand::Owner;


    std::string output = "I have troubles with database connection. Please try again later.";

    bool found = false;

    unsigned int currentAccess = 0;

    if (searchUser(_database, originalMessage->from, &found, &currentAccess))
    {
        if (found)
        {
            if (access > currentAccess)
            {
                std::string oldAccessString = ChatCommand::getAccessString(currentAccess);

                if (updateUser(_database, originalMessage->from, access))
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
                output += ChatCommand::getAccessString(currentAccess);
                output += "* which is higher or equal than *";
                output += ChatCommand::getAccessString(access);
                output += "*.";
            }
            else
            {
                output = "You've entered wrong code.\nYour current access level is *";
                output += ChatCommand::getAccessString(currentAccess);
                output += "*.";
            }
        }
        else
        {
            if (access > ChatCommand::Default)
            {
                if (createUser(_database, originalMessage->from, access))
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
    }

    _telegram.sendMessage(originalMessage->from->id, output, TelegramBot::Markdown);
    return;
}

void Application::handleGetCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    std::string filter;

    if (command.size() > 1)
        filter = command[1];

    _telegram.sendMessage(originalMessage->from->id, Utilities::treeToString(_settings, filter));
}

void Application::handleSetCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
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

void Application::handleLoadCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    std::string answerString = loadConfiguration() ? 
        "The configuration has been successfully reloaded" : "The configuration reload failed";

    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleSaveCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    std::string answerString = saveConfiguration() ? 
        "The configuration has been successfully saved" : "The configuration save failed";

    _telegram.sendMessage(originalMessage->from->id, answerString);
}

void Application::handleWhereCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    if (_lastValidGpsMessage.imei.empty() || !_lastValidGpsMessage.validPosition)
    {
        _telegram.sendMessage(originalMessage->from->id, "Unfortunately there's no valid location received from tracker.");
        return;
    }

    _telegram.sendLocation(originalMessage->from->id, (float) _lastValidGpsMessage.latitude, (float) _lastValidGpsMessage.longitude);

    std::string output;

    output += "Host Time: *" + posix_time::to_simple_string(_lastValidGpsMessage.hostTime) + "*\n";
    output += "Tracker Time: " + posix_time::to_simple_string(_lastValidGpsMessage.trackerTime) + "\n";
    output += "Speed: " + lexical_cast<std::string>(_lastValidGpsMessage.speed) + "\n";
    output += "Phone: *" + _lastValidGpsMessage.phone + "*\n";
    output += "Status: " + _lastValidGpsMessage.keyword + "\n";

    _telegram.sendMessage(originalMessage->from->id, output);
}

void Application::handleStatusCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    if (_lastGpsMessage.imei.empty())
    {
        _telegram.sendMessage(originalMessage->from->id, "Unfortunately there's no data received from tracker.");
        return;
    }

    if (_lastGpsMessage.validPosition)
        _telegram.sendLocation(originalMessage->from->id, (float) _lastGpsMessage.latitude, (float) _lastGpsMessage.longitude);

    std::string output;

    output += "Host Time: *" + posix_time::to_simple_string(_lastGpsMessage.hostTime) + "*\n";
    output += "Tracker Time: " + posix_time::to_simple_string(_lastGpsMessage.trackerTime) + "\n";
    output += "Speed: " + lexical_cast<std::string>(_lastGpsMessage.speed) + "\n";
    output += "Phone: *" + _lastGpsMessage.phone + "*\n";
    output += "Status: " + _lastGpsMessage.keyword + "\n";

    _telegram.sendMessage(originalMessage->from->id, output);
}
