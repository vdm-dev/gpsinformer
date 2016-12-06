#include "StdAfx.h"

#include "Application.h"

void Application::startJabber()
{
    bool        enabled        = _settings.get("jabber.enabled", false);
    std::string jid            = _settings.get("jabber.jid", std::string());
    std::string password       = _settings.get("jabber.password", std::string());
    std::string host           = _settings.get("jabber.host", std::string());
    int         port           = _settings.get("jabber.port", 5222);

    if (!enabled)
    {
        BOOST_LOG_TRIVIAL(info) << "Jabber service is disabled";
        return;
    }

    // Dirty hack to access a const member
    const_cast<gloox::JID&>(_jabber.jid()).setJID(jid);

    _jabber.setPassword(password);
    _jabber.setServer(host);
    _jabber.setPort(port);
    _jabber.connectionImpl()->setServer(host, port);

    BOOST_LOG_TRIVIAL(info) << "Jabber service is connecting to <" << host << ">";

    _jabber.connect();
}

void Application::onConnect()
{
    BOOST_LOG_TRIVIAL(info) << "Jabber service connected successfully";
}

void Application::onDisconnect(gloox::ConnectionError error)
{
    if (error == gloox::ConnUserDisconnected)
    {
        BOOST_LOG_TRIVIAL(warning) << "Jabber service disconnected by user";
        return;
    }

    std::string message = "Jabber disconnected by peer";

    int64_t reconnectDelay = _settings.get("jabber.reconnect_delay", 0);

    system::error_code errorCode;

    _jabberTimer.expires_from_now(posix_time::milliseconds(reconnectDelay), errorCode);
    if (errorCode || (reconnectDelay < 1))
    {
        BOOST_LOG_TRIVIAL(warning) << message << " (no auto-reconnect)";
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << message << ". Trying to reconnect in " << (reconnectDelay / 1000.0) << " second(s)";

    _jabberTimer.async_wait(bind(&Application::handleJabberTimer, this, asio::placeholders::error));
}

bool Application::onTLSConnect(const gloox::CertInfo& info)
{
    return true;
}

void Application::handleJabberTimer(const system::error_code& error)
{
    startJabber();
}

void Application::handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message)
{
    if ((area == gloox::LogAreaXmlIncoming) || (area == gloox::LogAreaXmlOutgoing))
        return;

    switch (level)
    {
    case gloox::LogLevelDebug:
        BOOST_LOG_TRIVIAL(debug) << "Gloox: " << message;
        break;
    case gloox::LogLevelWarning:
        BOOST_LOG_TRIVIAL(warning) << "Gloox: " << message;
        break;
    default:
        BOOST_LOG_TRIVIAL(error) << "Gloox: " << message;
    }
}

void Application::handleMessage(const gloox::Message& message, gloox::MessageSession* session)
{
    int access = 0;

    BOOST_FOREACH(property_tree::ptree::value_type& participant, _settings.get_child("jabber.authorized"))
    {
        if (participant.first == "participant")
        {
            std::string jid = participant.second.get("jid", "");

            if (!jid.empty() && iequals(jid, message.from().bare()))
            {
                access = participant.second.get("access", 0);
                break;
            }
        }
    }

    std::string body = message.body();

    if (access < 1)
    {
        BOOST_LOG_TRIVIAL(warning) << "Ignored message from <" << message.from().full() << ">: " << body;
        return;
    }

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

                handleChatCommand(command, message, session);

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

    handleChatCommand(command, message, session);
}

void Application::handleSubscription(const gloox::Subscription& subscription)
{
    int access = 0;

    BOOST_FOREACH(property_tree::ptree::value_type& participant, _settings.get_child("jabber.authorized"))
    {
        if (participant.first == "participant")
        {
            std::string jid = participant.second.get("jid", "");

            if (!jid.empty() && iequals(jid, subscription.from().bare()))
            {
                access = participant.second.get("access", 0);
                break;
            }
        }
    }

    _jabber.rosterManager()->ackSubscriptionRequest(subscription.from(), access > 0);
}
