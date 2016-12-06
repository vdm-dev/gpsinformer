#ifndef GlooxTCPClient_INCLUDED
#define GlooxTCPClient_INCLUDED

class GlooxTCPClient : public gloox::ConnectionBase
{
public:
    GlooxTCPClient(asio::io_service& ioService, gloox::ConnectionDataHandler* handler = 0);
    virtual ~GlooxTCPClient();

    gloox::ConnectionError connect();
    void disconnect();

    gloox::ConnectionError recv(int timeout = -1);
    gloox::ConnectionError receive();

    bool send(const std::string& data);

    void cleanup();

    int localPort() const;
    const std::string localInterface() const;

    void getStatistics(long int &totalIn, long int &totalOut);

    gloox::ConnectionBase* newInstance() const;

private:
    void handleConnect(const system::error_code& error);
    void handleRead(size_t size, const system::error_code& error);
    void handleWrite(size_t size, const system::error_code& error);

    asio::io_service& _ioService;
    asio::ip::tcp::resolver _resolver;
    asio::ip::tcp::socket _socket;

    std::vector<char> _readBuffer;
    std::deque<std::string> _writeQueue;

    size_t _totalIn;
    size_t _totalOut;
};

#endif // GlooxTCPClient_INCLUDED
