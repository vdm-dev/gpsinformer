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

#include "SslClient.h"
#include "TcpClientHandler.h"


static void my_debug(void *ctx, int level,
    const char *file, int line,
    const char *str)
{
    ((void)level);

    BOOST_LOG_TRIVIAL(debug) << file << ":" << line << ":" << str;
}

int SslClient::sslReceiveCallback(void* context, unsigned char* buffer, size_t length)
{
    if (!context || !buffer || !length)
        return 0;

    SslClient* client = reinterpret_cast<SslClient*>(context);

    if (client->_status != SslClient::Handshake && client->_status != SslClient::Connected)
        return 0;

    if (client->_sslBuffer.size() == 0)
        return MBEDTLS_ERR_SSL_WANT_READ;

    int size = std::min(client->_sslBuffer.size(), length);

    memcpy(buffer, client->_sslBuffer.data(), size);

    client->_sslBuffer.erase(client->_sslBuffer.begin(), client->_sslBuffer.begin() + size);

    return size;
}

int SslClient::sslSendCallback(void* context, const unsigned char* buffer, size_t length)
{
    if (!context || !buffer || !length)
        return 0;

    SslClient* client = reinterpret_cast<SslClient*>(context);

    if (client->_status != SslClient::Handshake && client->_status != SslClient::Connected)
        return 0;

    client->_tcpClient.send(buffer, length);

    return (int) length;
}


SslClient::SslClient(asio::io_service& ioService, TcpClientHandler<SslClient>* handler)
    : _tcpClient(ioService)
    , _timer(ioService)
    , _timerRead(ioService)
    , _timerWrite(ioService)
    , _handler(handler)
    , _status(Disconnected)
    , _ioCount(0)
    , _wasConnected(false)
{
    _tcpClient.setEventHandler(this);
}

SslClient::~SslClient()
{
}

void SslClient::connect(const std::string& server, unsigned short port)
{
    connect(server, lexical_cast<std::string>(port));
}

void SslClient::connect(const std::string& server, const std::string& protocol)
{
    BOOST_LOG_TRIVIAL(debug) << "<SslClient> (st: " << _status << ", ios: " << _ioCount << ") connect(" << server << ", " << protocol << ")";

    if (_status != Disconnected)
        return;

    _server = server;

    mbedtls_debug_set_threshold(1);

    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_entropy_init(&entropy);

    int result;

    result = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 0, 0);
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(debug) << " failed\n  ! mbedtls_ctr_drbg_seed returned " << result;
        return;
    }


    result = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (result < 0)
    {
        BOOST_LOG_TRIVIAL(debug) << " failed\n  !  mbedtls_x509_crt_parse returned " << result;
        return;
    }

    _sslBuffer.clear();
    _writeBuffer.clear();

    _status = Connecting;

    _tcpClient.connect(server, protocol);
}

void SslClient::disconnect()
{
    BOOST_LOG_TRIVIAL(debug) << "<SslClient> (st: " << _status << ", ios: " << _ioCount << ") disconnect";

    if (_status != Disconnected && _status != Disconnecting && _status != Error)
    {
        _status = Disconnecting;
        _tcpClient.disconnect();
    }
}

void SslClient::send(const std::string& data)
{
    BOOST_LOG_TRIVIAL(debug) << "<SslClient> (st: " << _status << ", ios: " << _ioCount << ") send(size: " << data.length() << ")";

    if (_status != Connected)
        return;

    bool inProgress = !_writeBuffer.empty();

    _writeBuffer.push_back(data);

    if (!inProgress)
    {
        system::error_code errorCode;

        _timerWrite.expires_from_now(posix_time::milliseconds(1), errorCode);

        if (!errorCode)
        {
            _ioCount++;
            _timerWrite.async_wait(bind(&SslClient::handleWrite, this, asio::placeholders::error));
        }
    }
}

bool SslClient::handleAnything(Status handleStatus, const system::error_code& error)
{
    _ioCount--;

    if (_status == Disconnected)
        return false;

    if (_status == Disconnecting || _status == Error || error)
    {
        if (_status != Disconnecting)
            _status = Error;

        _tcpClient.disconnect();

        // Waiting for all IO operations to complete
        if (_ioCount > 0)
            return false;

        Status status = _status;

        _writeBuffer.clear();
        _status = Disconnected;
        _ioCount = 0;
        _wasConnected = false;

        if (!_handler)
            return false;

        if (handleStatus == Connected)
        {
            _handler->handleTcpClientDisconnect(this,
                status == Disconnecting ? TcpClientHandler<SslClient>::ClosedByUser : TcpClientHandler<SslClient>::ClosedByPeer);
        }
        else
        {
            _handler->handleTcpClientError(this,
                status == Disconnecting ? asio::error::make_error_code(asio::error::basic_errors::operation_aborted) : error);
        }

        return false;
    }

    return true;
}

void SslClient::handleHandshake(const system::error_code& error)
{
    BOOST_LOG_TRIVIAL(debug) << "<SslClient> (st: " << _status << ", ios: " << _ioCount - 1 << ") handleHandshake(" << error << ")";

    system::error_code errorCode;

    int result = mbedtls_ssl_handshake(&ssl);

    if (result != 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE)
        errorCode = system::error_code(result, asio::error::get_misc_category());

    if (!handleAnything(Handshake, errorCode))
        return;

    if (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
        _timer.expires_from_now(posix_time::milliseconds(1), errorCode);

        if (!errorCode)
        {
            _ioCount++;
            _timer.async_wait(bind(&SslClient::handleHandshake, this, asio::placeholders::error));
        }

        return;
    }

    uint32_t flags;

    BOOST_LOG_TRIVIAL(debug) << "Verifying peer X.509 certificate...";

    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        char vrfy_buf[512];

        BOOST_LOG_TRIVIAL(debug) << " failed ";

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

        BOOST_LOG_TRIVIAL(debug) << vrfy_buf;
    }
    else
        BOOST_LOG_TRIVIAL(debug) << " ok";

    _status = Connected;
    _wasConnected = true;

    if (_handler)
        _handler->handleTcpClientConnect(this);
    
    _timerRead.expires_from_now(posix_time::milliseconds(1), errorCode);

    if (!errorCode)
    {
        _ioCount++;
        _timerRead.async_wait(bind(&SslClient::handleRead, this, asio::placeholders::error));
    }
}

void SslClient::handleRead(const system::error_code& error)
{
    BOOST_LOG_TRIVIAL(debug) << "<SslClient> (st: " << _status << ", ios: " << _ioCount - 1 << ") handleRead(" << error << ")";

    system::error_code errorCode;

    char readBuffer[32768];

    int result = mbedtls_ssl_read(&ssl, (unsigned char*) readBuffer, sizeof(readBuffer));

    if (result <= 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE)
        errorCode = system::error_code(result ? result : 2, asio::error::get_misc_category());

    if (!handleAnything(Connected, errorCode))
        return;

    if (_handler && result > 0)
    {
        std::string data(readBuffer, result);

        _handler->handleTcpClientReceivedData(this, data);
    }

    _timerRead.expires_from_now(posix_time::milliseconds(1), errorCode);

    if (!errorCode)
    {
        _ioCount++;
        _timerRead.async_wait(bind(&SslClient::handleRead, this, asio::placeholders::error));
    }
}

void SslClient::handleWrite(const system::error_code& error)
{
    BOOST_LOG_TRIVIAL(debug) << "<SslClient> (st: " << _status << ", ios: " << _ioCount - 1 << ") handleWrite(" << error << ")";

    system::error_code errorCode;

    if (_writeBuffer.empty())
    {
        _ioCount--;
        return;
    }

    std::string data = _writeBuffer.front();

    int result = mbedtls_ssl_write(&ssl, (const unsigned char*) data.data(), data.size());

    if (result <= 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE)
        errorCode = system::error_code(result ? result : 2, asio::error::get_misc_category());

    if (!handleAnything(Connected, errorCode))
        return;

    if (result != MBEDTLS_ERR_SSL_WANT_WRITE && result != MBEDTLS_ERR_SSL_WANT_READ)
    {
        _writeBuffer.pop_front();
    }

    _timerWrite.expires_from_now(posix_time::milliseconds(1), errorCode);

    if (!errorCode)
    {
        _ioCount++;
        _timerWrite.async_wait(bind(&SslClient::handleWrite, this, asio::placeholders::error));
    }
}

void SslClient::handleTcpClientConnect(TcpClient* client)
{
    int result;

    result = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, 
        MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);

    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(debug) << " failed\n  ! mbedtls_ssl_config_defaults returned " << result;
        return;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

    result = mbedtls_ssl_setup(&ssl, &conf);
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(debug) << " failed\n  ! mbedtls_ssl_setup returned " << result;
        return;
    }

    result = mbedtls_ssl_set_hostname(&ssl, _server.c_str());
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(debug) << " failed\n  ! mbedtls_ssl_set_hostname returned " << result;
        return;
    }

    mbedtls_ssl_set_bio(&ssl, this, &SslClient::sslSendCallback, &SslClient::sslReceiveCallback, NULL);

    _status = Handshake;

    system::error_code errorCode;

    _timer.expires_from_now(posix_time::milliseconds(1), errorCode);

    if (!errorCode)
    {
        _ioCount++;
        _timer.async_wait(bind(&SslClient::handleHandshake, this, asio::placeholders::error));
    }
}

void SslClient::handleTcpClientDisconnect(TcpClient* client, TcpClientHandler::Reason reason)
{
    char readBuffer[32768];

    while (_wasConnected && _sslBuffer.size() != 0)
    {
        int result = mbedtls_ssl_read(&ssl, (unsigned char*) readBuffer, sizeof(readBuffer));
        if (result <= 0)
            break;

        if (_handler)
        {
            std::string data(readBuffer, result);

            _handler->handleTcpClientReceivedData(this, data);
        }

    }

    _ioCount++;
    handleAnything(_wasConnected ? Connected : Handshake, asio::error::make_error_code(asio::error::misc_errors::eof));
}

void SslClient::handleTcpClientError(TcpClient* client, const system::error_code& error)
{
    _ioCount++;
    handleAnything(Connecting, error);
}

void SslClient::handleTcpClientReceivedData(TcpClient* client, const std::string& data)
{
    _sslBuffer.insert(_sslBuffer.end(), data.begin(), data.end());
}
