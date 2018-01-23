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


void SslClient::sslDebugCallback(void *context, int level, const char *file, int line, const char *string)
{
    BOOST_LOG_TRIVIAL(debug) << file << ":" << line << ":" << string;
}

int SslClient::sslReadCallback(void* context, unsigned char* buffer, size_t length)
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

int SslClient::sslWriteCallback(void* context, const unsigned char* buffer, size_t length)
{
    if (!context || !buffer || !length)
        return 0;

    SslClient* client = reinterpret_cast<SslClient*>(context);

    if (client->_status != SslClient::Handshake && client->_status != SslClient::Connected)
        return 0;

    client->_tcpClient.send(buffer, length);

    return (int) length;
}


bool SslClient::sslInitialize()
{
    if (_sslInitialized)
        return true;

    mbedtls_ssl_init(&_sslContext);
    mbedtls_ssl_config_init(&_sslConfiguration);
    mbedtls_x509_crt_init(&_sslCaRoot);
    mbedtls_ctr_drbg_init(&_sslCtrDrbg);
    mbedtls_entropy_init(&_sslEntropy);

    int result;

    result = mbedtls_ctr_drbg_seed(&_sslCtrDrbg, mbedtls_entropy_func, &_sslEntropy, 0, 0);
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(error) << "SSL Initialization failed: mbedtls_ctr_drbg_seed returned " << result;
        sslFree();
        return false;
    }

    result = mbedtls_x509_crt_parse(&_sslCaRoot, (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (result < 0)
    {
        BOOST_LOG_TRIVIAL(error) << "SSL Initialization failed: mbedtls_x509_crt_parse returned " << result;
        sslFree();
        return false;
    }

    result = mbedtls_ssl_config_defaults(&_sslConfiguration, MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(error) << "SSL Initialization failed: mbedtls_ssl_config_defaults returned " << result;
        sslFree();
        return false;
    }

    mbedtls_ssl_conf_authmode(&_sslConfiguration, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&_sslConfiguration, &_sslCaRoot, NULL);
    mbedtls_ssl_conf_rng(&_sslConfiguration, mbedtls_ctr_drbg_random, &_sslCtrDrbg);
    mbedtls_ssl_conf_dbg(&_sslConfiguration, &SslClient::sslDebugCallback, 0);

    result = mbedtls_ssl_setup(&_sslContext, &_sslConfiguration);
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(error) << "SSL Initialization failed: mbedtls_ssl_setup returned " << result;
        sslFree();
        return false;
    }

    mbedtls_ssl_set_bio(&_sslContext, this, &SslClient::sslWriteCallback, &SslClient::sslReadCallback, NULL);

    _sslInitialized = true;

    return true;
}

void SslClient::sslFree()
{
    if (_sslInitialized)
    {
        mbedtls_entropy_free(&_sslEntropy);
        mbedtls_ctr_drbg_free(&_sslCtrDrbg);
        mbedtls_x509_crt_free(&_sslCaRoot);
        mbedtls_ssl_config_free(&_sslConfiguration);
        mbedtls_ssl_free(&_sslContext);

        _sslInitialized = false;
    }
}

SslClient::SslClient(asio::io_service& ioService, TcpClientHandler<SslClient>* handler)
    : _tcpClient(ioService)
    , _timer(ioService)
    , _handler(handler)
    , _status(Disconnected)
    , _ioCount(0)
    , _sslInitialized(false)
    , _wasConnected(false)
{
    _tcpClient.setEventHandler(this);
}

SslClient::~SslClient()
{
    sslFree();
}

void SslClient::connect(const std::string& server, unsigned short port)
{
    connect(server, lexical_cast<std::string>(port));
}

void SslClient::connect(const std::string& server, const std::string& protocol)
{
    if (!sslInitialize())
        return;

    if (_status != Disconnected)
        return;

    int result = mbedtls_ssl_session_reset(&_sslContext);
    if (result != 0)
    {
        BOOST_LOG_TRIVIAL(error) << "SSL Initialization failed: mbedtls_ssl_session_reset returned " << result;
        return;
    }

    // Avoid memory leakage during every connect call
    if (_sslContext.hostname == 0 || server.compare(_sslContext.hostname) != 0)
    {
        result = mbedtls_ssl_set_hostname(&_sslContext, server.c_str());
        if (result != 0)
        {
            BOOST_LOG_TRIVIAL(error) << "SSL Initialization failed: mbedtls_ssl_set_hostname returned " << result;
            return;
        }
    }

    _sslBuffer.clear();
    _writeBuffer.clear();

    _status = Connecting;

    _tcpClient.connect(server, protocol);
}

void SslClient::disconnect()
{
    if (_status != Disconnected && _status != Disconnecting && _status != Error)
    {
        _status = Disconnecting;
        _tcpClient.disconnect();
    }
}

void SslClient::send(const std::string& data)
{
    if (_status != Connected)
        return;

    bool inProgress = !_writeBuffer.empty();

    _writeBuffer.push_back(data);

    if (!inProgress)
    {
        _ioCount++;
        handleWrite();
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

void SslClient::handleHandshake()
{
    system::error_code error;

    int result = mbedtls_ssl_handshake(&_sslContext);

    if (result != 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE)
        error = system::error_code(result, asio::error::get_misc_category());

    if (!handleAnything(Handshake, error))
        return;

    if (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE)
        return;

    uint32_t flags = mbedtls_ssl_get_verify_result(&_sslContext);
    if (flags != 0)
    {
        char message[512];

        if (mbedtls_x509_crt_verify_info(message, sizeof(message), "", flags) > 0)
            BOOST_LOG_TRIVIAL(debug) << "SSL Warning: " << message;
    }

    _status = Connected;
    _wasConnected = true;

    if (_handler)
        _handler->handleTcpClientConnect(this);
}

bool SslClient::handleRead()
{
    system::error_code error;

    char readBuffer[32768];

    int result = mbedtls_ssl_read(&_sslContext, (unsigned char*) readBuffer, sizeof(readBuffer));

    if (result <= 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
        error = system::error_code(
            result ? result : asio::error::misc_errors::eof, asio::error::get_misc_category());
    }

    if (!handleAnything(Connected, error))
        return false;

    if (_handler && result > 0)
    {
        std::string data(readBuffer, result);

        _handler->handleTcpClientReceivedData(this, data);
    }

    return (result > 0);
}

void SslClient::handleWrite()
{
    system::error_code error;

    if (_writeBuffer.empty())
    {
        _ioCount--;
        return;
    }

    std::string data = _writeBuffer.front();

    int result = mbedtls_ssl_write(&_sslContext, (const unsigned char*) data.data(), data.size());

    if (result <= 0 && result != MBEDTLS_ERR_SSL_WANT_READ && result != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
        error = system::error_code(
            result ? result : asio::error::misc_errors::eof, asio::error::get_misc_category());
    }

    if (!handleAnything(Connected, error))
        return;

    if (result != MBEDTLS_ERR_SSL_WANT_WRITE && result != MBEDTLS_ERR_SSL_WANT_READ)
        _writeBuffer.pop_front();

    _timer.expires_from_now(posix_time::milliseconds(1), error);

    if (!error)
    {
        _ioCount++;
        _timer.async_wait(bind(&SslClient::handleWrite, this));
    }
}

void SslClient::handleTcpClientConnect(TcpClient* client)
{
    _status = Handshake;

    _ioCount++;
    handleHandshake();
}

void SslClient::handleTcpClientDisconnect(TcpClient* client, TcpClientHandler::Reason reason)
{
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

    if (_status == Handshake)
    {
        _ioCount++;
        handleHandshake();
    }
    else
    {
        while (_status == Connected)
        {
            _ioCount++;
            if (!handleRead())
                break;
        }
    }
}
