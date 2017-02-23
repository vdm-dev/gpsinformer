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