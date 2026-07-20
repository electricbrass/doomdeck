#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

import std;
import subprocess;

TEST_CASE("Capture stdout", "[subprocess]") {
    auto result = subprocess::run_command("sh", "-c", "printf hello");

    REQUIRE(result.has_value());
    REQUIRE(result->stdout == "hello");
    REQUIRE(result->exit_code == 0);
}

TEST_CASE("Capture stderr", "[subprocess]") {
    auto result = subprocess::run_command("sh", "-c", "printf error >&2");

    REQUIRE(result.has_value());
    REQUIRE(result->stderr == "error");
}

TEST_CASE("Return exit code", "[process]") {
    auto result = subprocess::run_command("sh", "-c", "exit 42");

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 42);
}
