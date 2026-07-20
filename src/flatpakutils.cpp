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

export module flatpakutils;

import std;

import stringutils;
import subprocess;
import utils;

export namespace flatpakutil {

using Version = util::Version;

auto version() -> std::optional<Version> {
    const auto result = subprocess::run_command("flatpak", "--version");
    if (result.has_value()) {
        if (result->exit_code != 0) {
            return std::nullopt;
        }

        auto trimmed = stringutil::trim(result.value().stdout);
        static constexpr std::string_view flatpak_prefix = "Flatpak ";
        trimmed.remove_prefix(flatpak_prefix.size());
        const auto parts = stringutil::split(trimmed, '.');
        if (parts.size() != 3) {
            return std::nullopt;
        }
        const auto major = util::parse_num<std::uint32_t>(parts[0]);
        const auto minor = util::parse_num<std::uint32_t>(parts[1]);
        const auto patch = util::parse_num<std::uint32_t>(parts[2]);
        if (!major || !minor || !patch) {
            return std::nullopt;
        }
        return Version{.major = *major, .minor = *minor, .patch = *patch};
    }

    return std::nullopt;
}

}; // namespace flatpakutil
