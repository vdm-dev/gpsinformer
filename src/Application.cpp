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

#include "Application.h"

#include <sqlite3.h>
#include <mbedtls/debug.h>


Application* Application::_instance = 0;

Application::Application()
    : _ioService()
    , _receiverTimer(_ioService)
    , _transmitterTimer(_ioService)
    , _receiver(_ioService)
    , _transmitter(_ioService)
    , _telegram(_ioService)
    , _database(0)
    , _transmitterAuthorized(false)
{
    _instance = this;

    // Avoid C4355
    _receiver.setEventHandler(this);
    _transmitter.setEventHandler(this);
    _telegram.setEventHandler(this);
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

    openDatabase();
    dbRestoreSettings();

    fillCommandList();

    startReceiver();
    startTransmitter();
    startTelegram();

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
#if (BOOST_VERSION >= 105600)
        property_tree::write_xml(_configurationFile.string(), _settings, std::locale(),
            property_tree::xml_writer_make_settings<std::string>(' ', 2));
#else
        property_tree::write_xml(_configurationFile.string(), _settings, std::locale(),
            property_tree::xml_writer_make_settings<char>(' ', 2));
#endif
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
    int         ssl_level      = _settings.get("log.ssl_level", 0);

    mbedtls_debug_set_threshold(ssl_level);


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
