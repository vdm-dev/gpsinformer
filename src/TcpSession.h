#ifndef TcpSession_INCLUDED
#define TcpSession_INCLUDED


class TcpServer;
class TcpSessionHandler;


class TcpSession : public enable_shared_from_this<TcpSession>
{
public:
    TcpSession(asio::io_service& ioService, TcpServer& server, TcpSessionHandler* handler = 0);
    ~TcpSession();

    void cleanup();
    void close();
    void disconnect();
    void send(const std::string& data);
    void start();

    asio::ip::tcp::socket& socket();

    bool isActive() const;

private:
    void read();
    void write();

    void handleRead(size_t size, const system::error_code& error);
    void handleWrite(size_t size, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::socket _socket;

    std::vector<char> _readBuffer;
    std::deque<std::string> _writeQueue;

    bool _active;

    TcpServer& _server;

    TcpSessionHandler* _handler;
};


inline asio::ip::tcp::socket& TcpSession::socket()
{
    return _socket;
}

inline bool TcpSession::isActive() const
{
    return _active;
}


#endif // TcpSession_INCLUDED
