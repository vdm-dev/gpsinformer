cmake_minimum_required (VERSION 2.8)

set (PROJECT gpsinformer)

project (${PROJECT})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -std=c++11 -Wall")

add_definitions (-DBOOST_LOG_DYN_LINK)

include_directories (dep/sqlite3/include src)

set (SOURCES
	dep/sqlite3/sqlite3.c
	src/tgbot/TgTypeParser.cpp
	src/tgbot/types/InlineQueryResult.cpp
	src/tgbot/types/InputFile.cpp
	src/Application.cpp
	src/ApplicationCommands.cpp
	src/ApplicationDatabase.cpp
	src/ApplicationReceiver.cpp
	src/ApplicationTelegram.cpp
	src/ApplicationTransmitter.cpp
	src/Device.cpp
	src/HttpParser.cpp
	src/HttpRequest.cpp
	src/HttpsClient.cpp
	src/Main.cpp
	src/SslClient.cpp
	src/StdAfx.cpp
	src/TcpClient.cpp
	src/TcpServer.cpp
	src/TcpSession.cpp
	src/TelegramBot.cpp
	src/Url.cpp
	src/Utilities.cpp
)

find_package (Threads REQUIRED)

find_package (OpenSSL REQUIRED)
include_directories (${OPENSSL_INCLUDE_DIR})

set (Boost_USE_MULTITHREADED ON)

find_package (Boost REQUIRED COMPONENTS system log log_setup date_time thread filesystem program_options REQUIRED)
include_directories (${Boost_INCLUDE_DIR})

set (LIB_LIST
	${CMAKE_THREAD_LIBS_INIT}
	${OPENSSL_LIBRARIES}
	${Boost_LIBRARIES}
)

add_executable (${PROJECT} ${SOURCES})

target_link_libraries (${PROJECT} ${LIB_LIST} -ldl)