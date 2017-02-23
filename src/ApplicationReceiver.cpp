#include "StdAfx.h"

#include "Application.h"


void Application::startReceiver()
{
    bool           enabled           = _settings.get("receiver.enabled", false);
    std::string    host              = _settings.get("receiver.host", std::string("0.0.0.0"));
    unsigned short port              = _settings.get("receiver.port", 9090);

    if (!enabled)
    {
        BOOST_LOG_TRIVIAL(info) << "Receiver is disabled";
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Receiver is listening on <" << host << ":" << port << ">";

    _receiver.listen(host, port);
}

void Application::handleTcpServerError(const system::error_code& error)
{
    _devices.clear();

    std::string message = "Receiver error: " + error.message();

    int64_t rebindDelay = _settings.get("receiver.rebind_delay", 0);

    system::error_code errorCode;

    _receiverTimer.expires_from_now(posix_time::milliseconds(rebindDelay), errorCode);
    if (errorCode || (rebindDelay < 1))
    {
        BOOST_LOG_TRIVIAL(warning) << message << " (no auto-rebind)";
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << message << ". Trying to rebind in " << (rebindDelay / 1000.0) << " second(s)";

    _receiverTimer.async_wait(bind(&Application::handleReceiverTimer, this, asio::placeholders::error));
}

void Application::handleTcpSessionConnect(shared_ptr<TcpSession> session)
{
    _devices[session] = make_shared<Device>(session);

    system::error_code error;

    std::string address = "unknown";

    asio::ip::tcp::endpoint endpoint = session->socket().remote_endpoint(error);
    if (!error)
        address = endpoint.address().to_string(error);

    BOOST_LOG_TRIVIAL(info) << "Client <" << address << "> connected";
}

void Application::handleTcpSessionDisconnect(shared_ptr<TcpSession> session, TcpSessionHandler::Reason reason)
{
    if (!session->isActive())
        return;

    _devices.erase(session);

    if (reason == TcpSessionHandler::ClosedByServer)
        return;

    system::error_code error;

    std::string address = "unknown";

    asio::ip::tcp::endpoint endpoint = session->socket().remote_endpoint(error);
    if (!error)
        address = endpoint.address().to_string(error);

    BOOST_LOG_TRIVIAL(info) << "Client <" << address << "> disconnected";
}

void Application::handleTcpSessionReceivedData(shared_ptr<TcpSession> session, const std::string& data)
{
    if (_devices.count(session))
        _devices[session]->processData(data);
}

void Application::handleReceiverTimer(const system::error_code& error)
{
    startReceiver();
}
