namespace Utilities
{


std::string treeToString(const property_tree::ptree& tree, const std::string& filter = "", const std::string& path = "");
bool wildcardMatch(const std::string string, const std::string pattern, size_t sOffset = 0, size_t pOffset = 0);


} // namespace Utilities