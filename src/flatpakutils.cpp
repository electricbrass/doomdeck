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

#include <glaze/glaze.hpp>

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

struct FlatpakApp {
    std::string name;
    std::string app_id;
    std::unordered_set<std::string> branches;
    std::string current_branch;

    constexpr auto operator==(const FlatpakApp&) const -> bool = default;
};

// exported for unit testing purposes
namespace detail {

struct FlatpakJSON {
    std::string name;
    std::string application_id;
    std::string branch;
    std::string options;

    // For some reason Catch2 doesn't like an implicit operator==
    constexpr auto operator==(const FlatpakJSON&) const -> bool = default;
};

auto parse_apps_from_json(const std::string_view json) -> std::vector<FlatpakJSON> {
    std::vector<FlatpakJSON> parsed;
    if (const auto errors = glz::read_json(parsed, json)) {
        // TODO: error handling!!
    }
    return parsed;
}

auto parse_apps_from_columns(const std::string_view columns) -> std::vector<FlatpakJSON> {
    using std::views::transform, std::views::split, std::views::filter, std::ranges::to;
    using stringutil::trim, util::filter_transform;
    return columns | split('\n') | filter_transform([](auto&& line) -> std::optional<FlatpakJSON> {
               const auto columns = stringutil::split(trim(std::string_view{line}), '\t');
               if (columns.size() != 4) {
                   return std::nullopt;
               }
               return FlatpakJSON{
                   .name{columns[0]},
                   .application_id{columns[1]},
                   .branch{columns[2]},
                   .options{columns[3]},
               };
           }) |
           to<std::vector<FlatpakJSON>>();
}

auto apps_json_to_whatever(const std::span<const FlatpakJSON> parsed_json)
    -> std::unordered_map<std::string, FlatpakApp> {
    std::unordered_map<std::string, FlatpakApp> apps;
    for (const auto& ref : parsed_json) {
        auto& app = apps[ref.application_id];

        if (app.name.empty()) {
            app.name = ref.name;
            app.app_id = ref.application_id;
        }

        app.branches.insert(ref.branch);

        if (ref.options.contains("current")) {
            app.current_branch = ref.branch;
        }
    }
    return apps;
}

} // namespace detail

// Use this one with flatpak 1.17.0 or newer
auto installed_apps_from_json() -> std::unordered_map<std::string, FlatpakApp> {
    const auto result = subprocess::run_command(
        "flatpak", "list", "--app", "--json", "--columns=name,application,branch,options"
    );
    if (!result || result->exit_code != 0) {
        return {};
    }
    return detail::apps_json_to_whatever(detail::parse_apps_from_json(result->stdout));
}

// Use this one with flatpak older than 1.17.0
auto installed_apps_from_columns() -> std::unordered_map<std::string, FlatpakApp> {
    const auto result = subprocess::run_command(
        "flatpak", "list", "--app", "--columns=name,application,branch,options"
    );
    if (!result || result->exit_code != 0) {
        return {};
    }
    return detail::apps_json_to_whatever(detail::parse_apps_from_columns(result->stdout));
}

}; // namespace flatpakutil
