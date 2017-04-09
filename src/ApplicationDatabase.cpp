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
#include "GpsMessage.h"
#include "ChatCommand.h"
#include "User.h"

#include <sqlite3.h>


void Application::openDatabase()
{
    int result = 0;

    const char* sqlUsers = "CREATE TABLE IF NOT EXISTS users ( \
                                id  INTEGER NOT NULL PRIMARY KEY, \
                                firstname TEXT NOT NULL, \
                                lastname TEXT NOT NULL, \
                                nickname TEXT NOT NULL, \
                                access INTEGER NOT NULL DEFAULT 0)";

    const char* sqlData = "CREATE TABLE IF NOT EXISTS data ( \
                                imei TEXT NOT NULL, \
                                keyword TEXT NOT NULL, \
                                phone TEXT NOT NULL, \
                                tracker_time INTEGER NOT NULL DEFAULT 0, \
                                host_time INTEGER NOT NULL DEFAULT 0, \
                                latitude REAL NOT NULL DEFAULT 0.0, \
                                longitude REAL NOT NULL DEFAULT 0.0, \
                                speed REAL NOT NULL DEFAULT 0.0, \
                                valid INTEGER NOT NULL DEFAULT 0)";


    std::string fileName = _settings.get("database.file", std::string());

    if (fileName.empty())
    {
        filesystem::path filePath = _applicationPath;
        filePath.replace_extension(".db");

        fileName = filePath.string();
    }

    result = sqlite3_open(fileName.c_str(), &_database);
    if (result != SQLITE_OK)
        goto error;


    result = sqlite3_exec(_database, sqlUsers, 0, 0, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_exec(_database, sqlData, 0, 0, 0);
    if (result != SQLITE_OK)
        goto error;

    return;

error:
    BOOST_LOG_TRIVIAL(error) << "Database error: " << sqlite3_errmsg(_database);
    sqlite3_close(_database);
    _database = 0;
}

void Application::closeDatabase()
{
    sqlite3_close(_database);
    _database = 0;
}

bool Application::dbAddGpsData(const GpsMessage& data)
{
    using namespace boost::posix_time;

    int result = 0;
    sqlite3_stmt *stmt = 0;

    uint64_t trackerTime = 0;
    uint64_t hostTime = 0;
    int valid = 0;

    const char* sql = "INSERT INTO data (imei, keyword, phone, tracker_time, host_time, latitude, longitude, speed, valid) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    result = sqlite3_prepare_v2(_database, sql, -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 1, data.imei.c_str(), data.imei.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 2, data.keyword.c_str(), data.keyword.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 3, data.phone.c_str(), data.phone.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    if (!data.trackerTime.is_special())
        trackerTime = to_time_t(data.trackerTime);

    result = sqlite3_bind_int64(stmt, 4, trackerTime);
    if (result != SQLITE_OK)
        goto error;

    if (!data.hostTime.is_special())
        hostTime = to_time_t(data.hostTime);

    result = sqlite3_bind_int64(stmt, 5, hostTime);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_double(stmt, 6, data.latitude);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_double(stmt, 7, data.longitude);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_double(stmt, 8, data.speed);
    if (result != SQLITE_OK)
        goto error;

    valid = data.validPosition ? 1 : 0;

    result = sqlite3_bind_int(stmt, 9, valid);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);
    if ((result != SQLITE_OK) && (result != SQLITE_DONE))
        goto error;

    sqlite3_finalize(stmt);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(_database);
    return false;

}

bool Application::dbCreateUser(const User& user)
{
    int result = 0;
    sqlite3_stmt *stmt = 0;

    const char* sql = "INSERT INTO users (id, firstname, lastname, nickname, access) VALUES (?, ?, ?, ?, ?)";

    if (!user.valid || !user.tg)
        return false;

    result = sqlite3_prepare_v2(_database, sql, -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 1, user.tg->id);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 2, user.tg->firstName.c_str(), user.tg->firstName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 3, user.tg->lastName.c_str(), user.tg->lastName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 4, user.tg->username.c_str(), user.tg->username.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 5, user.access);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);
    if ((result != SQLITE_OK) && (result != SQLITE_DONE))
        goto error;

    sqlite3_finalize(stmt);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(_database);
    return false;
}

bool Application::dbGetGpsData(std::vector<GpsMessage>& data, unsigned int limit, bool validOnly)
{
    int result = 0;
    sqlite3_stmt *stmt = 0;

    GpsMessage gpsMessage;

    std::string sql = "SELECT * FROM data";

    data.clear();

    if (validOnly)
        sql += " WHERE valid = 1";

    if (limit)
        sql += " LIMIT ?";

    result = sqlite3_prepare_v2(_database, sql.c_str(), -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    if (limit)
    {
        result = sqlite3_bind_int(stmt, 1, limit);
        if (result != SQLITE_OK)
            goto error;
    }

    result = sqlite3_step(stmt);

    while (result == SQLITE_ROW)
    {
        int columns = sqlite3_column_count(stmt);

        for (int column = 0; column < columns; ++column)
        {
            std::string columnName = sqlite3_column_name(stmt, column);

            if (columnName == "imei")
            {
                gpsMessage.imei = (const char*) sqlite3_column_text(stmt, column);
            }
            else if (columnName == "keyword")
            {
                gpsMessage.keyword = (const char*) sqlite3_column_text(stmt, column);
            }
            else if (columnName == "phone")
            {
                gpsMessage.phone = (const char*) sqlite3_column_text(stmt, column);
            }
            else if (columnName == "tracker_time")
            {
                uint64_t value = sqlite3_column_int64(stmt, column);

                if (value)
                    gpsMessage.trackerTime = posix_time::from_time_t(value);
            }
            else if (columnName == "host_time")
            {
                uint64_t value = sqlite3_column_int64(stmt, column);

                if (value)
                    gpsMessage.hostTime = posix_time::from_time_t(value);
            }
            else if (columnName == "latitude")
            {
                gpsMessage.latitude = sqlite3_column_double(stmt, column);
            }
            else if (columnName == "longitude")
            {
                gpsMessage.longitude = sqlite3_column_double(stmt, column);
            }
            else if (columnName == "speed")
            {
                gpsMessage.speed = sqlite3_column_double(stmt, column);
            }
            else if (columnName == "valid")
            {
                gpsMessage.validPosition = sqlite3_column_int(stmt, column) != 0;
            }
        }

        data.push_back(gpsMessage);

        result = sqlite3_step(stmt);
    }

    if ((result != SQLITE_DONE) && (result != SQLITE_OK))
        goto error;

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK)
        goto error;

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(_database);
    return false;
}

bool Application::dbSearchUser(User& user)
{
    int result = 0;
    sqlite3_stmt *stmt = 0;

    const char* sql = "SELECT * FROM users WHERE id = ? LIMIT 1";

    bool needUpdate = false;

    user.access = ChatCommand::Default;
    user.valid = false;

    if (!user.tg)
        return false;

    result = sqlite3_prepare_v2(_database, sql, -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 1, user.tg->id);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);

    while (result == SQLITE_ROW)
    {
        int columns = sqlite3_column_count(stmt);

        for (int column = 0; column < columns; ++column)
        {
            std::string columnName = sqlite3_column_name(stmt, column);

            if (columnName == "firstname")
            {
                std::string text = (const char*) sqlite3_column_text(stmt, column);
                needUpdate |= (text != user.tg->firstName);
            }
            else if (columnName == "lastname")
            {
                std::string text = (const char*) sqlite3_column_text(stmt, column);
                needUpdate |= (text != user.tg->lastName);
            }
            else if (columnName == "nickname")
            {
                std::string text = (const char*) sqlite3_column_text(stmt, column);
                needUpdate |= (text != user.tg->username);
            }
            else if (columnName == "access")
            {
                user.access = sqlite3_column_int(stmt, column);
            }

            user.valid = true;
        }

        result = sqlite3_step(stmt);
    }

    if ((result != SQLITE_DONE) && (result != SQLITE_OK))
        goto error;

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK)
        goto error;

    if (needUpdate)
        dbUpdateUser(user);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(_database);
    return false;
}

bool Application::dbUpdateUser(const User& user)
{
    int result = 0;
    sqlite3_stmt *stmt = 0;

    const char* sql = "UPDATE users SET firstname = ?, lastname = ?, nickname = ?, access = ? WHERE id = ?";

    if (!user.valid || !user.tg)
        return false;

    result = sqlite3_prepare_v2(_database, sql, -1, &stmt, 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 1, user.tg->firstName.c_str(), user.tg->firstName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 2, user.tg->lastName.c_str(), user.tg->lastName.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_text(stmt, 3, user.tg->username.c_str(), user.tg->username.size(), 0);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 4, user.access);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_bind_int(stmt, 5, user.tg->id);
    if (result != SQLITE_OK)
        goto error;

    result = sqlite3_step(stmt);
    if ((result != SQLITE_OK) && (result != SQLITE_DONE))
        goto error;

    sqlite3_finalize(stmt);

    return true;

error:
    BOOST_LOG_TRIVIAL(debug) << "Database error: " << sqlite3_errmsg(_database);
    return false;
}