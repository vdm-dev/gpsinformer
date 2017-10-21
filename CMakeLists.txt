cmake_minimum_required (VERSION 2.8)

set (PROJECT gpsinformer)

project (${PROJECT})

if (NOT MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif ()

if (UNIX OR CYGWIN)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")	
endif ()


# Uncomment this if Boost static linking will fail
# add_definitions (-DBOOST_LOG_DYN_LINK)

if (MSVC)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif ()


set (SQLITE3_SOURCES
	dep/sqlite3/sqlite3.c
)

set (SQLITE3_HEADERS
	dep/sqlite3/include/sqlite3.h
	dep/sqlite3/include/sqlite3ext.h
)

set (TGBOT_SOURCES
	src/tgbot/TgTypeParser.cpp
)

set (TGBOT_HEADERS
	src/tgbot/TgTypeParser.h
)

set (TGBOT_TYPES_SOURCES
	src/tgbot/types/InlineQueryResult.cpp
	src/tgbot/types/InputFile.cpp
)

set (TGBOT_TYPES_HEADERS
	src/tgbot/types/Audio.h
	src/tgbot/types/CallbackQuery.h
	src/tgbot/types/Chat.h
	src/tgbot/types/ChatMember.h
	src/tgbot/types/ChosenInlineResult.h
	src/tgbot/types/Contact.h
	src/tgbot/types/Document.h
	src/tgbot/types/File.h
	src/tgbot/types/ForceReply.h
	src/tgbot/types/GenericReply.h
	src/tgbot/types/InlineKeyboardButton.h
	src/tgbot/types/InlineKeyboardMarkup.h
	src/tgbot/types/InlineQuery.h
	src/tgbot/types/InlineQueryResult.h
	src/tgbot/types/InlineQueryResultArticle.h
	src/tgbot/types/InlineQueryResultAudio.h
	src/tgbot/types/InlineQueryResultCachedAudio.h
	src/tgbot/types/InlineQueryResultCachedDocument.h
	src/tgbot/types/InlineQueryResultCachedGif.h
	src/tgbot/types/InlineQueryResultCachedMpeg4Gif.h
	src/tgbot/types/InlineQueryResultCachedPhoto.h
	src/tgbot/types/InlineQueryResultCachedSticker.h
	src/tgbot/types/InlineQueryResultCachedVideo.h
	src/tgbot/types/InlineQueryResultCachedVoice.h
	src/tgbot/types/InlineQueryResultContact.h
	src/tgbot/types/InlineQueryResultDocument.h
	src/tgbot/types/InlineQueryResultGame.h
	src/tgbot/types/InlineQueryResultGif.h
	src/tgbot/types/InlineQueryResultLocation.h
	src/tgbot/types/InlineQueryResultMpeg4Gif.h
	src/tgbot/types/InlineQueryResultPhoto.h
	src/tgbot/types/InlineQueryResultVenue.h
	src/tgbot/types/InlineQueryResultVideo.h
	src/tgbot/types/InlineQueryResultVoice.h
	src/tgbot/types/InputContactMessageContent.h
	src/tgbot/types/InputFile.h
	src/tgbot/types/InputLocationMessageContent.h
	src/tgbot/types/InputMessageContent.h
	src/tgbot/types/InputTextMessageContent.h
	src/tgbot/types/InputVenueMessageContent.h
	src/tgbot/types/KeyboardButton.h
	src/tgbot/types/Location.h
	src/tgbot/types/Message.h
	src/tgbot/types/MessageEntity.h
	src/tgbot/types/PhotoSize.h
	src/tgbot/types/ReplyKeyboardMarkup.h
	src/tgbot/types/ReplyKeyboardRemove.h
	src/tgbot/types/ResponseParameters.h
	src/tgbot/types/Sticker.h
	src/tgbot/types/Update.h
	src/tgbot/types/User.h
	src/tgbot/types/UserProfilePhotos.h
	src/tgbot/types/Venue.h
	src/tgbot/types/Video.h
	src/tgbot/types/Voice.h
	src/tgbot/types/WebhookInfo.h
)

set (SOURCES
	src/Application.cpp
	src/ApplicationCommands.cpp
	src/ApplicationDatabase.cpp
	src/ApplicationEvents.cpp
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

set (HEADERS
	src/Application.h
	src/ChatCommand.h
	src/Device.h
	src/GpsMessage.h
	src/HttpClientHandler.h
	src/HttpParser.h
	src/HttpReqArg.h
	src/HttpRequest.h
	src/HttpsClient.h
	src/SslClient.h
	src/StdAfx.h
	src/TcpClient.h
	src/TcpClientHandler.h
	src/TcpServer.h
	src/TcpSession.h
	src/TcpSessionHandler.h
	src/TelegramBot.h
	src/TelegramBotHandler.h
	src/Url.h
	src/User.h
	src/UserSettings.h
	src/Utilities.h
)

# Threads
find_package (Threads REQUIRED)


# OpenSSL
if (WIN32)
	set (OPENSSL_USE_STATIC_LIBS FALSE)
	set (OPENSSL_ROOT_DIR dep/openssl)
endif ()

find_package (OpenSSL REQUIRED)


# Boost
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
find_package (Boost REQUIRED COMPONENTS system log log_setup date_time thread filesystem program_options REQUIRED)


include_directories (dep/sqlite3/include src)
include_directories (${OPENSSL_INCLUDE_DIR})
include_directories (${Boost_INCLUDE_DIR})

link_directories (${Boost_LIBRARY_DIRS})


set (LIB_LIST
	${CMAKE_THREAD_LIBS_INIT}
	${OPENSSL_LIBRARIES}
)

# Visual Studio supports auto-linking of Boost Libraries
if (NOT MSVC)
	set (LIB_LIST
		${LIB_LIST}
		${Boost_LIBRARIES}
	)
endif ()

if (UNIX AND NOT CYGWIN)
	set (LIB_LIST
		${LIB_LIST}
		-ldl
	)
endif ()


set (FULL_SOURCES
	${SQLITE3_SOURCES}
	${TGBOT_SOURCES}
	${TGBOT_TYPES_SOURCES}
	${SOURCES}
)

set (FULL_HEADERS
	${SQLITE3_HEADERS}
	${TGBOT_HEADERS}
	${TGBOT_TYPES_HEADERS}
	${HEADERS}
)


source_group ("Dependencies\\SQLite 3" FILES ${SQLITE3_SOURCES} ${SQLITE3_HEADERS})
source_group ("Dependencies\\TgBot" FILES ${TGBOT_SOURCES} ${TGBOT_HEADERS})
source_group ("Dependencies\\TgBot\\Types" FILES ${TGBOT_TYPES_SOURCES} ${TGBOT_TYPES_HEADERS})
source_group ("Header Files" FILES ${HEADERS})
source_group ("Source Files" FILES ${SOURCES})


add_executable (${PROJECT} ${FULL_SOURCES} ${FULL_HEADERS})

target_link_libraries (${PROJECT} ${LIB_LIST})