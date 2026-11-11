#include "lq/release.h"

#include <algorithm>

namespace lq::release {

namespace {

constexpr std::string_view kMarker = "REPLACE_ME";

bool isAsciiAlnum(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

bool hasMarker(std::string_view name)
{
    return name.find(kMarker) != std::string_view::npos;
}

} // namespace

// GitHub's own rules for a user or organization name.
bool isValidOwner(std::string_view owner)
{
    if (owner.empty() || owner.size() > 39 || hasMarker(owner) || owner.front() == '-' || owner.back() == '-')
        return false;
    return std::all_of(owner.begin(), owner.end(), [](char c) { return isAsciiAlnum(c) || c == '-'; });
}

// GitHub's own rules for a repository name. ".git" is what a pasted clone address ends in.
bool isValidRepo(std::string_view repo)
{
    if (repo.empty() || repo.size() > 100 || hasMarker(repo) || repo == "." || repo == ".." || repo.ends_with(".git"))
        return false;
    return std::all_of(repo.begin(), repo.end(),
                       [](char c) { return isAsciiAlnum(c) || c == '-' || c == '_' || c == '.'; });
}

std::string latestReleasePage(std::string_view owner, std::string_view repo)
{
    if (!isValidOwner(owner) || !isValidRepo(repo))
        return {};
    std::string page = "https://github.com/";
    page += owner;
    page += '/';
    page += repo;
    page += "/releases/latest";
    return page;
}

std::string latestReleasePage()
{
    return latestReleasePage(kOwner, kRepo);
}

} // namespace lq::release
