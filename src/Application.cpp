#include "StdAfx.h"

#include "Application.h"
#include "GlooxTCPClient.h"

Application* Application::_instance = 0;

Application::Application()
    : _jabberTimer(_ioService)
    , _receiverTimer(_ioService)
    , _transmitterTimer(_ioService)
    , _jabber("")
    , _receiver(_ioService)
    , _transmitter(_ioService)
    , _transmitterAuthorized(false)
{
    _instance = this;

    _jabber.setConnectionImpl(new GlooxTCPClient(_ioService, &_jabber));
    _jabber.registerConnectionListener(this);
    _jabber.logInstance().registerLogHandler(gloox::LogLevelDebug, gloox::LogAreaAll, this);
    _jabber.registerMessageHandler(this);
    _jabber.registerSubscriptionHandler(this);

    // Avoid C4355
    _receiver.setEventHandler(this);
    _transmitter.setEventHandler(this);
}

Application::~Application()
{
    if (_instance == this)
        _instance = 0;
}

Application* Application::instance()
{
    return _instance;
}

int Application::main(int argc, char* argv[])
{
    if (!setApplicationPath(argc, argv) || !parseCommandLine(argc, argv))
        return -1;

    if (_configurationFile.empty())
    {
        _configurationFile = _applicationPath;
        _configurationFile.replace_extension(".xml");
    }

    if (!loadConfiguration())
        return -1;

    configureLog();

    startJabber();
    startReceiver();
    startTransmitter();

    system::error_code error;

    _ioService.run(error);

    return 0;
}

bool Application::parseCommandLine(int argc, char* argv[])
{
    using namespace std;
    namespace po = program_options;

    po::options_description description("Options");

    description.add_options()
        ("help", "Show usage information")
        ("configuration,c", po::value<string>()->implicit_value(""), "Path to configuration file")
    ;

    po::variables_map variables;

    po::store(po::parse_command_line(argc, argv, description), variables);
    po::notify(variables);

    if (variables.count("help"))
    {
        cout << description << endl;
        return false;
    }

    if (variables.count("configuration"))
        _configurationFile = variables["configuration"].as<string>();
    
    return true;
}

bool Application::setApplicationPath(int argc, char* argv[])
{
#ifdef _WIN32
    wchar_t path[1025];

    wmemset(path, 0, boost::size(path));

    if (!GetModuleFileNameW(NULL, path, boost::size(path) - 1))
        return false;

    _applicationPath = path;
#else
    if (argc)
        _applicationPath = argv[0];
#endif // _WIN32

    return !_applicationPath.empty();
}

bool Application::loadConfiguration()
{
    try
    {
        property_tree::read_xml(_configurationFile.string(), _settings, 
            property_tree::xml_parser::trim_whitespace);
    }
    catch (const property_tree::xml_parser_error&)
    {
        return false;
    }

    return true;
}

bool Application::saveConfiguration()
{
    try
    {
        property_tree::write_xml(_configurationFile.string(), _settings, std::locale(),
            property_tree::xml_writer_make_settings<std::string>(' ', 2));
    }
    catch (const property_tree::xml_parser_error&)
    {
        return false;
    }

    return true;
}

void Application::configureLog()
{
    std::string dateTimeFormat = _settings.get("log.date_time_format", "%d-%m-%Y, %H:%M:%S");
    std::string file           = _settings.get("log.file", "");
    uintmax_t   rotationSize   = _settings.get("log.rotation_size", 0);
    int         level          = _settings.get("log.level", 0);

    if (!rotationSize)
        rotationSize = std::numeric_limits<uintmax_t>::max();

    log::formatter formatter = log::expressions::stream
        << "[" << log::expressions::format_date_time<posix_time::ptime>("TimeStamp", dateTimeFormat) << "] "
        << "<" << log::trivial::severity << "> "
        << log::expressions::message;

    if (!file.empty())
    {
        log::add_file_log
        (
            log::keywords::auto_flush = true,
            log::keywords::file_name = file,
            log::keywords::rotation_size = rotationSize
        )->set_formatter(formatter);
    }

    log::add_console_log()->set_formatter(formatter);

    log::core::get()->set_filter(log::trivial::severity >= level);

    log::add_common_attributes();
}

void Application::handleDeviceCommand(const std::vector<std::string>& arguments)
{
    std::string command = algorithm::join(arguments, ",");
    BOOST_LOG_TRIVIAL(debug) << "Receiver obtains data: " << command;

    if (_transmitterAuthorized)
    {
        _transmitter.send(command + ";\r\n");
    }
}
