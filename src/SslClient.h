#ifndef SslClient_INCLUDED
#define SslClient_INCLUDED


template <class T>
class TcpClientHandler;


class SslClient
{
public:
    SslClient(asio::io_service& ioService, TcpClientHandler<SslClient>* handler = 0);
    ~SslClient();

    void connect(const std::string& server, unsigned short port);
    void disconnect(bool byUser = true);

    void cleanup();

    void send(const std::string& data);

    void setEventHandler(TcpClientHandler<SslClient>* handler);

    asio::ssl::stream<asio::ip::tcp::socket>& socket();

private:
    void handleConnect(const system::error_code& error);
    void handleHandshake(const system::error_code& error);
    void handleRead(size_t size, const system::error_code& error);
    void handleResolve(const system::error_code& error, asio::ip::tcp::resolver::iterator endpoint);
    void handleWrite(size_t size, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ssl::context _context;
    asio::ssl::stream<asio::ip::tcp::socket> _socket;

    std::vector<char> _readBuffer;
    std::deque<std::string> _writeQueue;

    TcpClientHandler<SslClient>* _handler;
};


inline void SslClient::setEventHandler(TcpClientHandler<SslClient>* handler)
{
    _handler = handler;
}

inline asio::ssl::stream<asio::ip::tcp::socket>& SslClient::socket()
{
    return _socket;
}


#endif // SslClient_INCLUDED
