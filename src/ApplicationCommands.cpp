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

/*

void Application::handleChatCommand(const std::vector<std::string>& command, const gloox::Message& message, gloox::MessageSession* session)
{
    if (!command.size())
        return;

    if (iequals(command[0], "get"))
    {
        handleGetCommand(command, message.from());
    }
    else if (iequals(command[0], "set"))
    {
        handleSetCommand(command, message.from());
    }
    else if (iequals(command[0], "load"))
    {
        handleLoadCommand(command, message.from());
    }
    else if (iequals(command[0], "save"))
    {
        handleSaveCommand(command, message.from());
    }
}

void Application::handleGetCommand(const std::vector<std::string>& command, const gloox::JID& sender)
{
    std::string filter;

    if (command.size() > 1)
        filter = command[1];

    gloox::Message answer(gloox::Message::Chat, sender, Utilities::treeToString(_settings, filter));
    _jabber.send(answer);
}

void Application::handleSetCommand(const std::vector<std::string>& command, const gloox::JID& sender)
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
        answerString = "Usage: set <key> <value> [!]";
    }

    gloox::Message answer(gloox::Message::Chat, sender, answerString);
    _jabber.send(answer);
}

void Application::handleLoadCommand(const std::vector<std::string>& command, const gloox::JID& sender)
{
    std::string answerString = loadConfiguration() ? 
        "The configuration has been successfully reloaded" : "The configuration reload failed";

    gloox::Message answer(gloox::Message::Chat, sender, answerString);
    _jabber.send(answer);
}

void Application::handleSaveCommand(const std::vector<std::string>& command, const gloox::JID& sender)
{
    std::string answerString = saveConfiguration() ? 
        "The configuration has been successfully saved" : "The configuration save failed";

    gloox::Message answer(gloox::Message::Chat, sender, answerString);
    _jabber.send(answer);
}

*/
