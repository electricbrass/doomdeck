/*
 * Copyright (C) 2026  Mia McMahill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

module;

#include <pwd.h>
#include <unistd.h>

export module shellutils;

import std;

namespace fs = std::filesystem;

export namespace shellutil {

auto parse_command_line(std::string_view line) -> std::vector<std::string> {
    std::vector<std::string> result;
    std::ispanstream in(line);
    return result;
}

auto expand_home_dir(const fs::path& path) -> fs::path {
    if (path.empty() || *path.begin() != "~") {
        return path;
    }

    static const auto home_dir = []() {
        if (const char* home = std::getenv("HOME")) {
            if (*home != '\0') {
                return fs::path(home);
            }
        }

        if (const auto pwuid = getpwuid(getuid())) {
            if (pwuid->pw_dir && *pwuid->pw_dir != '\0') {
                return fs::path(pwuid->pw_dir);
            }
        }

        return fs::path{};
    }();

    if (home_dir.empty()) {
        return path;
    }

    const auto path_str = path.string();

    if (path_str == "~") {
        return home_dir;
    }

    return home_dir / path_str.substr(2);
}

} // namespace shellutil
