#ifndef StdAfx_INCLUDED
#define StdAfx_INCLUDED

#ifdef _WIN32
#include <sdkddkver.h>
#endif


#include <cstdio>
#include <cstdint>
#include <cwchar>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/range.hpp>


using namespace boost;


#endif // StdAfx_INCLUDED
