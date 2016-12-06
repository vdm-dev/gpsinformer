#include "StdAfx.h"

#include "GlooxTCPClient.h"

const std::string gloox::EmptyString = "";

GlooxTCPClient::GlooxTCPClient(asio::io_service& ioService, gloox::ConnectionDataHandler* handler)
    : gloox::ConnectionBase(handler)
    , _ioService(ioService)
    , _resolver(ioService)
    , _socket(ioService)
    , _readBuffer(4096)
    , _totalIn(0)
    , _totalOut(0)
{
}


GlooxTCPClient::~GlooxTCPClient()
{
}

gloox::ConnectionError GlooxTCPClient::connect()
{
    m_state = gloox::StateConnecting;

    asio::ip::tcp::resolver::query query(m_server, lexical_cast<std::string>(m_port));

    system::error_code error;

    asio::ip::tcp::resolver::iterator endpoint = _resolver.resolve(query, error);

    if (error)
    {
        m_state = gloox::StateDisconnected;

        return gloox::ConnDnsError;
    }

    asio::async_connect(_socket, endpoint, boost::bind(&GlooxTCPClient::handleConnect, this, asio::placeholders::error));

    return gloox::ConnNoError;
}

void GlooxTCPClient::disconnect()
{
    if (m_handler)
        m_handler->handleDisconnect(this, gloox::ConnUserDisconnected);

    cleanup();
}

gloox::ConnectionError GlooxTCPClient::recv(int timeout)
{
    return gloox::ConnNoError;
}

gloox::ConnectionError GlooxTCPClient::receive()
{
    return gloox::ConnNoError;
}

bool GlooxTCPClient::send(const std::string& data)
{
    if (!_socket.is_open())
        return false;

    bool inProgress = !_writeQueue.empty();

    _writeQueue.push_back(data);

    if (!inProgress)
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&GlooxTCPClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }

    return true;
}

void GlooxTCPClient::cleanup()
{
    m_state = gloox::StateDisconnected;

    system::error_code error;
    _socket.close(error);

    _totalIn = 0;
    _totalOut = 0;
}

int GlooxTCPClient::localPort() const
{
    system::error_code error;
    const asio::ip::tcp::endpoint& endpoint = _socket.local_endpoint(error);

    return error ? -1 : endpoint.port();
}

const std::string GlooxTCPClient::localInterface() const
{
    system::error_code error;
    const asio::ip::tcp::endpoint& endpoint = _socket.local_endpoint(error);

    return error ? std::string() : endpoint.address().to_string(error);
}

void GlooxTCPClient::getStatistics(long int &totalIn, long int &totalOut)
{
    totalIn = _totalIn;
    totalOut = _totalOut;
}

gloox::ConnectionBase* GlooxTCPClient::newInstance() const
{
    return 0;
}

void GlooxTCPClient::handleConnect(const system::error_code& error)
{
    if (error)
    {
        if (m_handler)
            m_handler->handleDisconnect(this, gloox::ConnConnectionRefused);

        cleanup();
        return;
    }

    m_state = gloox::StateConnected;

    if (m_handler)
        m_handler->handleConnect(this);

    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&GlooxTCPClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void GlooxTCPClient::handleRead(size_t size, const system::error_code& error)
{
    if (error || !size)
    {
        if (m_handler)
            m_handler->handleDisconnect(this, gloox::ConnStreamError);

        cleanup();
        return;
    }

    _totalIn += size;

    std::string data(_readBuffer.begin(), _readBuffer.begin() + size);

    if (m_handler)
        m_handler->handleReceivedData(this, data);

    asio::async_read(_socket, asio::buffer(_readBuffer, _readBuffer.size()), asio::transfer_at_least(1),
        boost::bind(&GlooxTCPClient::handleRead, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
}

void GlooxTCPClient::handleWrite(size_t size, const system::error_code& error)
{
    if (error)
    {
        if (m_handler)
            m_handler->handleDisconnect(this, gloox::ConnStreamError);

        cleanup();
        return;
    }

    _totalOut += size;

    _writeQueue.pop_front();

    if (!_writeQueue.empty())
    {
        asio::async_write(_socket, asio::buffer(_writeQueue.front().data(), _writeQueue.front().length()),
            boost::bind(&GlooxTCPClient::handleWrite, this, asio::placeholders::bytes_transferred, asio::placeholders::error));
    }
}
