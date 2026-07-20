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

TEST_CASE("Return exit code", "[subprocess]") {
    auto result = subprocess::run_command("sh", "-c", "exit 42");

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 42);
}

TEST_CASE("Command does not exist", "[subprocess]") {
    auto result = subprocess::run_command("nonexistent-really-long-command-so-this-doesnt-accidentally-succeed");

    REQUIRE(!result.has_value());
    REQUIRE(result.error() == subprocess::SubprocessError::NotFound);
}

TEST_CASE("Source port does not exist", "[subprocess]") {
    std::array<std::string, 1> args = {"nonexistent-really-long-command-so-this-doesnt-accidentally-succeed"};
    auto result = subprocess::launch_game(args);

    REQUIRE(!result.has_value());
    REQUIRE(result.error() == subprocess::SubprocessError::NotFound);
}

TEST_CASE("File is not executable", "[subprocess]") {
    const auto path = std::filesystem::temp_directory_path() / "doomdeck-not-executable";

    {
        std::ofstream file(path);
        file << "#!/bin/sh\nexit 0\n";
    }

    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_read |
        std::filesystem::perms::owner_write
    );
    auto result = subprocess::run_command(path.string());

    REQUIRE(!result.has_value());
    REQUIRE(result.error() == subprocess::SubprocessError::PermissionDenied);

    std::filesystem::remove(path);
}
