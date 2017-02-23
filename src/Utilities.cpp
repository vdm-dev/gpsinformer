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

#include "Utilities.h"


namespace Utilities
{


std::string treeToString(const property_tree::ptree& tree, const std::string& filter, const std::string& path)
{
    std::string result;

    BOOST_FOREACH(const property_tree::ptree::value_type& branch, tree)
    {
        std::string currentPath = path.empty() ? branch.first : path + "." + branch.first;

        if (branch.second.empty())
        {
            if (filter.empty() || (currentPath.find(filter) == 0) || wildcardMatch(currentPath, filter))
                result += currentPath + " = \"" + branch.second.get_value("") + "\"\r\n";
        }
        else
        {
            result += treeToString(branch.second, filter, currentPath);
        }
    }

    return result;
}

bool wildcardMatch(const std::string string, const std::string pattern, size_t sOffset, size_t pOffset)
{
    while (pOffset < pattern.size())
    {
        if (pattern[pOffset] == '?')
        {
            if (sOffset >= string.size())
                return false;

            ++sOffset;
            ++pOffset;
        }
        else if (pattern[pOffset] == '*')
        {
            if (wildcardMatch(string, pattern, sOffset, pOffset + 1))
                return true;

            if ((sOffset < string.size()) && wildcardMatch(string, pattern, sOffset + 1, pOffset))
                return true;

            return false;
        }
        else
        {
            if (toupper(string[sOffset]) != toupper(pattern[pOffset]))
                return false;

            ++sOffset;
            ++pOffset;
        }
    }

    return (sOffset >= string.size()) && (pOffset >= pattern.size());
}


} // namespace Utilities