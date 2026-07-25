#include <catch2/catch_test_macros.hpp>

import std;
import utils;

TEST_CASE("Number parsing", "[utils]") {
    // Just basic int test
    REQUIRE(util::parse_num<std::uint32_t>("42").value() == 42);
    // Negative numbers
    REQUIRE(util::parse_num<std::uint32_t>("-42") == std::nullopt);
    REQUIRE(util::parse_num<std::int32_t>("-42").value() == -42);
    // Trailing characters
    REQUIRE(util::parse_num<std::uint32_t>("42a") == std::nullopt);
    // Leading characters
    REQUIRE(util::parse_num<std::uint32_t>("a42") == std::nullopt);
    // Floating point
    REQUIRE(util::parse_num<std::uint32_t>("42.5") == std::nullopt);
    REQUIRE(util::parse_num<float>("42.5").value() == 42.5f);
    REQUIRE(util::parse_num<float>("-42.5").value() == -42.5f);
    // Out of range
    REQUIRE(util::parse_num<std::uint8_t>("256") == std::nullopt);
}

TEST_CASE("Version comparison", "[utils]") {
    using util::Version;

    // Equal
    REQUIRE(Version{1, 0, 0} == Version{1, 0, 0});
    // Major
    REQUIRE(Version{2, 0, 0} > Version{1, 0, 0});
    REQUIRE(Version{1, 0, 0} < Version{2, 0, 0});
    // Minor
    REQUIRE(Version{1, 0, 0} < Version{1, 1, 0});
    REQUIRE(Version{1, 1, 0} > Version{1, 0, 0});
    // Patch
    REQUIRE(Version{1, 0, 0} < Version{1, 0, 1});
    REQUIRE(Version{1, 0, 1} > Version{1, 0, 0});
    // Other
    REQUIRE(Version{2, 0, 0} > Version{1, 99, 99});
    REQUIRE(Version{1, 2, 0} > Version{1, 1, 99});
    REQUIRE(Version{0, 0, 0} < Version{0, 0, 1});
    REQUIRE(Version{0, 1, 0} > Version{0, 0, 99});
}
