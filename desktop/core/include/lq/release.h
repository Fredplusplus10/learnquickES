#pragma once

#include <string>
#include <string_view>

// Where LearnQuick's downloads live: the only place in the code that names the public GitHub repository.
namespace lq::release {

// The public repository. Without a valid name here the download button stays off and the test `release-config`
// fails, saying what is missing.
inline constexpr std::string_view kOwner = "Fredplusplus10";
inline constexpr std::string_view kRepo = "learnquickES";

bool isValidOwner(std::string_view owner);
bool isValidRepo(std::string_view repo);

// https://github.com/<owner>/<repo>/releases/latest, or empty when either name is not valid.
std::string latestReleasePage(std::string_view owner, std::string_view repo);
// The same for the repository named above.
std::string latestReleasePage();

} // namespace lq::release
