#ifndef TCPClient_INCLUDED
#define TCPClient_INCLUDED

class TCPClientHandler;

class TCPClient
{
public:
    TCPClient(asio::io_service& ioService, TCPClientHandler* handler = 0);
    ~TCPClient();

    void connect(const std::string& server, unsigned short port);
    void disconnect(bool byUser = true);

    void cleanup();

    void send(const std::string& data);

    void setEventHandler(TCPClientHandler* handler);

    asio::ip::tcp::socket& socket();

private:
    void handleConnect(const system::error_code& error);
    void handleRead(size_t size, const system::error_code& error);
    void handleWrite(size_t size, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::socket _socket;

    std::vector<char> _readBuffer;
    std::deque<std::string> _writeQueue;

    TCPClientHandler* _handler;
};

inline void TCPClient::setEventHandler(TCPClientHandler* handler)
{
    _handler = handler;
}

inline asio::ip::tcp::socket& TCPClient::socket()
{
    return _socket;
}

#endif // TCPClient_INCLUDED
