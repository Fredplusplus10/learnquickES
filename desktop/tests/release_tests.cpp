// The one place that names the public repository, and the page the download button opens.
#include "lq/release.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

using namespace lq::release;

TEST_CASE("the latest-release page is built from a valid owner and repository", "[release]")
{
    CHECK(latestReleasePage("octo-cat", "learn.quick_1") == "https://github.com/octo-cat/learn.quick_1/releases/latest");
    CHECK(latestReleasePage("a", "b") == "https://github.com/a/b/releases/latest");
}

TEST_CASE("the marker, or a name that would change the address, gives no page", "[release]")
{
    CHECK(latestReleasePage("REPLACE_ME_GITHUB_USER", "REPLACE_ME_GITHUB_REPO").empty());
    CHECK(latestReleasePage("octo-cat", "REPLACE_ME_GITHUB_REPO").empty());
    CHECK(latestReleasePage("REPLACE_ME_GITHUB_USER", "learnquick").empty());

    for (const std::string_view owner : {"", "-octo", "octo-", "octo_cat", "octo/cat", "octo cat", "octo.cat", "octo@example.com"}) {
        INFO(owner);
        CHECK_FALSE(isValidOwner(owner));
    }
    CHECK_FALSE(isValidOwner(std::string(40, 'a')));
    CHECK(isValidOwner(std::string(39, 'a')));

    for (const std::string_view repo : {"", ".", "..", "repo.git", "a/b", "a b", "a?b=1", "a#b", "%2e%2e", "ñandú"}) {
        INFO(repo);
        CHECK_FALSE(isValidRepo(repo));
    }
    CHECK_FALSE(isValidRepo(std::string(101, 'a')));
    CHECK(isValidRepo(std::string(100, 'a')));
}

// Hidden from the `core` run and registered as its own ctest, `release-config`: while the repository does not exist
// this one fails on purpose, and nothing else has to go red with it.
TEST_CASE("the public repository is filled in", "[.][release-config]")
{
    if (!isValidOwner(kOwner) || !isValidRepo(kRepo)) {
        FAIL("LearnQuick does not name its public GitHub repository yet.\n"
             "\n"
             "WHAT IS MISSING: kOwner and kRepo in desktop/core/include/lq/release.h\n"
             "  they now hold: \"" << kOwner << "\" / \"" << kRepo << "\"\n"
             "\n"
             "WHY IT MATTERS: without them the \"Download latest version\" button in Settings has no page to open and\n"
             "stays off, so a build made like this cannot send a learner to the downloads.\n"
             "\n"
             "WHAT TO DO: once the public repository exists, put its user and name in those two constants.\n"
             "\n"
             "This failure is expected until then and is about this one setting: the app still builds and runs, and\n"
             "the other tests do not depend on it. To leave it out while working on something else:\n"
             "  ctest --test-dir build/desktop -LE release-gate");
    }
    CHECK(latestReleasePage() == "https://github.com/" + std::string(kOwner) + "/" + std::string(kRepo) + "/releases/latest");
}
