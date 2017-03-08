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


struct ChatCommand
{
    typedef void(Application::*ChatCommandHandler)(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage);

    enum Access
    {
        Default = 0,
        User = 1,
        Owner = 2
    };

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


void Application::fillCommandList()
{
    chatCommands.clear();

    // Basic
    chatCommands.push_back(ChatCommand("/start", "begin interaction with me", ChatCommand::Default, &Application::handleStartCommand));
    chatCommands.push_back(ChatCommand("/help", "show the list of commands available for you", ChatCommand::Default, &Application::handleHelpCommand));

    // Settings
    //chatCommands.push_back(ChatCommand("/get", "get current settings tree or branch", ChatCommand::Owner, &Application::handleGetCommand));
    //chatCommands.push_back(ChatCommand("/set", "set new settings", ChatCommand::Owner, &Application::handleSetCommand));
    chatCommands.push_back(ChatCommand("/load", "reload settings from file", ChatCommand::Owner, &Application::handleLoadCommand));
    chatCommands.push_back(ChatCommand("/save", "save current settings to file", ChatCommand::Owner, &Application::handleSaveCommand));
}

void Application::handleChatCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    if (command.empty())
        return;

    for (size_t i = 0; i < chatCommands.size(); ++i)
    {
        if (chatCommands[i].command.empty())
            continue;

        if (iequals(command[0], chatCommands[i].command) && chatCommands[i].handler)
            (this->*(chatCommands[i].handler))(command, originalMessage);
    }
}

void Application::handleStartCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
}

void Application::handleHelpCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr originalMessage)
{
    std::string output = "You can control me by sending these commands:\n\n";

    for (size_t i = 0; i < chatCommands.size(); ++i)
    {
        if (chatCommands[i].command.empty())
            continue;

        output += chatCommands[i].command + " - " + chatCommands[i].description + "\n";
    }

    _telegram.sendMessage(originalMessage->from->id, output);
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
