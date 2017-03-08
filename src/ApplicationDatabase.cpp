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


void Application::openDatabase()
{
    std::string fileName = _settings.get("database.file", std::string());

    if (fileName.empty())
    {
        filesystem::path filePath = _applicationPath;
        filePath.replace_extension(".db");

        fileName = filePath.string();
    }

    int result = sqlite3_open(fileName.c_str(), &_database);

    if (result != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Database error: " << sqlite3_errmsg(_database);
        sqlite3_close(_database);
        _database = 0;

        return;
    }

    std::string sql = "CREATE TABLE IF NOT EXISTS users ( \
                          id  INTEGER NOT NULL PRIMARY KEY, \
                          firstname TEXT NOT NULL, \
                          lastname TEXT NOT NULL, \
                          nickname TEXT NOT NULL, \
                          access INTEGER NOT NULL DEFAULT 0)";

    result = sqlite3_exec(_database, sql.c_str(), 0, 0, 0);
}

void Application::closeDatabase()
{
    sqlite3_close(_database);
    _database = 0;
}