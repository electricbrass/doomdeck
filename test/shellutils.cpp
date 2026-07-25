#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>

import std;
import shellutils;

namespace fs = std::filesystem;

TEST_CASE("Expand home directory", "[shellutils]") {
    setenv("HOME", "/home/test", 1);
    REQUIRE(shellutil::expand_home_dir("~") == fs::path{"/home/test"});
    REQUIRE(shellutil::expand_home_dir("~/foo") == fs::path{"/home/test/foo"});
    REQUIRE(shellutil::expand_home_dir("idk~") == fs::path{"idk~"});
    REQUIRE(shellutil::expand_home_dir("/middle/~/path") == fs::path{"/middle/~/path"});
    REQUIRE(shellutil::expand_home_dir("~~") == fs::path{"~~"});
    REQUIRE(shellutil::expand_home_dir("") == fs::path{});
}
