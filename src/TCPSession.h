#ifndef TCPSession_INCLUDED
#define TCPSession_INCLUDED

class TCPServer;
class TCPSessionHandler;

class TCPSession : public enable_shared_from_this<TCPSession>
{
public:
    TCPSession(asio::io_service& ioService, TCPServer& server, TCPSessionHandler* handler = 0);
    ~TCPSession();

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

    TCPServer& _server;

    TCPSessionHandler* _handler;
};

inline asio::ip::tcp::socket& TCPSession::socket()
{
    return _socket;
}

inline bool TCPSession::isActive() const
{
    return _active;
}

#endif // TCPSession_INCLUDED
