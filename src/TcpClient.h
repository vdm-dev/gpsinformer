#ifndef TcpClient_INCLUDED
#define TcpClient_INCLUDED


class TcpClientHandler;


class TcpClient
{
public:
    TcpClient(asio::io_service& ioService, TcpClientHandler* handler = 0);
    ~TcpClient();

    void connect(const std::string& server, unsigned short port);
    void disconnect(bool byUser = true);

    void cleanup();

    void send(const std::string& data);

    void setEventHandler(TcpClientHandler* handler);

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

    TcpClientHandler* _handler;
};


inline void TcpClient::setEventHandler(TcpClientHandler* handler)
{
    _handler = handler;
}

inline asio::ip::tcp::socket& TcpClient::socket()
{
    return _socket;
}


#endif // TcpClient_INCLUDED
