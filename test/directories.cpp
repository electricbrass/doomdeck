#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <stdlib.h>

import std;
import directories;
import errors;

class DirectoriesTestFixture {
public:
    DirectoriesTestFixture() {
        setenv("HOME", "/home/test", 1);
        unsetenv("XDG_BIN_HOME");
        unsetenv("XDG_CACHE_HOME");
        unsetenv("XDG_CONFIG_HOME");
        unsetenv("XDG_DATA_HOME");
        unsetenv("XDG_STATE_HOME");
        unsetenv("XDG_RUNTIME_HOME");
        unsetenv("XDG_CONFIG_DIRS");
        unsetenv("XDG_DATA_DIRS");
    }
};

using Catch::Matchers::RangeEquals;
using PathVec = std::vector<std::filesystem::path>;

TEST_CASE_METHOD(DirectoriesTestFixture, "All XDG vars unset", "[directories]") {
    const auto dirs = dirs::Directories();
    REQUIRE(dirs.cache_home() == "/home/test/.cache");
    REQUIRE(dirs.config_home() == "/home/test/.config");
    REQUIRE(dirs.data_home() == "/home/test/.local/share");
    REQUIRE(dirs.state_home() == "/home/test/.local/state");
    REQUIRE_THAT(dirs.data_dirs(), RangeEquals(PathVec{"/usr/local/share", "/usr/share"}));
    REQUIRE_THAT(dirs.data_search_dirs(),
                 RangeEquals(PathVec{"/home/test/.local/share", "/usr/local/share", "/usr/share"}));
}

TEST_CASE_METHOD(DirectoriesTestFixture, "Reject relative paths", "[directories]") {
    setenv("XDG_CACHE_HOME", "relative", 1);
    setenv("XDG_CONFIG_HOME", "./relative", 1);
    setenv("XDG_DATA_HOME", "../relative", 1);
    setenv("XDG_STATE_HOME", "relative/dir", 1);
    setenv("XDG_DATA_DIRS",
           "relative:/absolute:/absolute/too:./relative:/absolute/again:relative/too", 1);
    const auto dirs = dirs::Directories();
    REQUIRE(dirs.cache_home() == "/home/test/.cache");
    REQUIRE(dirs.config_home() == "/home/test/.config");
    REQUIRE(dirs.data_home() == "/home/test/.local/share");
    REQUIRE(dirs.state_home() == "/home/test/.local/state");
    REQUIRE_THAT(dirs.data_dirs(),
                 RangeEquals(PathVec{"/absolute", "/absolute/too", "/absolute/again"}));
    REQUIRE_THAT(dirs.data_search_dirs(),
                 RangeEquals(PathVec{"/home/test/.local/share", "/absolute", "/absolute/too",
                                     "/absolute/again"}));

    setenv("HOME", "relative", 1);
    REQUIRE_THROWS_AS(dirs::Directories(), errors::RuntimeError);
}

TEST_CASE_METHOD(DirectoriesTestFixture, "Reject empty paths", "[directories]") {
    setenv("XDG_CACHE_HOME", "", 1);
    setenv("XDG_CONFIG_HOME", "", 1);
    setenv("XDG_DATA_HOME", "", 1);
    setenv("XDG_STATE_HOME", "", 1);
    setenv("XDG_DATA_DIRS", "/absolute::/absolute/too", 1);
    auto dirs = dirs::Directories();
    REQUIRE(dirs.cache_home() == "/home/test/.cache");
    REQUIRE(dirs.config_home() == "/home/test/.config");
    REQUIRE(dirs.data_home() == "/home/test/.local/share");
    REQUIRE(dirs.state_home() == "/home/test/.local/state");
    REQUIRE_THAT(dirs.data_dirs(), RangeEquals(PathVec{"/absolute", "/absolute/too"}));
    REQUIRE_THAT(dirs.data_search_dirs(),
                 RangeEquals(PathVec{"/home/test/.local/share", "/absolute", "/absolute/too"}));
    setenv("XDG_DATA_DIRS", "", 1);
    dirs = dirs::Directories();
    REQUIRE_THAT(dirs.data_dirs(), RangeEquals(PathVec{"/usr/local/share", "/usr/share"}));
    setenv("XDG_DATA_DIRS", "::", 1);
    dirs = dirs::Directories();
    REQUIRE_THAT(dirs.data_dirs(), RangeEquals(PathVec{"/usr/local/share", "/usr/share"}));
}

TEST_CASE_METHOD(DirectoriesTestFixture, "All XDG vars set", "[directories]") {
    setenv("XDG_CACHE_HOME", "/cache", 1);
    setenv("XDG_CONFIG_HOME", "/config", 1);
    setenv("XDG_DATA_HOME", "/data", 1);
    setenv("XDG_STATE_HOME", "/state", 1);
    setenv("XDG_DATA_DIRS", "/data1:/data2", 1);
    const auto dirs = dirs::Directories();
    REQUIRE(dirs.cache_home() == "/cache");
    REQUIRE(dirs.config_home() == "/config");
    REQUIRE(dirs.data_home() == "/data");
    REQUIRE(dirs.state_home() == "/state");
    REQUIRE_THAT(dirs.data_dirs(), RangeEquals(PathVec{"/data1", "/data2"}));
    REQUIRE_THAT(dirs.data_search_dirs(), RangeEquals(PathVec{"/data", "/data1", "/data2"}));
}
