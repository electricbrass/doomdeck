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

namespace flatpakutil {
namespace {

constexpr const char* columns_arg = "--columns=name,application,branch,options,installation";

} // namespace
} // namespace flatpakutil

export namespace flatpakutil {

// TODO: function that takes flatpak stuff: app id, user vs system, branch, command
// and produces the args vector for run_command
// should it take in the rest of the args and handle the proper @@ and everything?
// or should it be on the caller to push_back those after getting the vector?

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
    enum struct Installation {
        User,
        System,
    };

    struct Branch {
        std::string name;
        Installation installation;
        bool is_current;

        // sorts system before user, the alphabetical by name
        constexpr auto operator<=>(const Branch& other) const -> std::strong_ordering {
            const auto installation_key = [](Installation i) {
                return i == Installation::System ? 0 : 1;
            };

            if (const auto cmp =
                    installation_key(installation) <=> installation_key(other.installation);
                cmp != std::strong_ordering::equal) {
                return cmp;
            }

            return name <=> other.name;
        }

        constexpr auto operator==(const Branch&) const -> bool = default;
    };

    std::string name;
    std::string app_id;
    std::vector<Branch> branches;

    constexpr auto operator==(const FlatpakApp&) const -> bool = default;
};

// exported for unit testing purposes
namespace detail {

struct FlatpakListEntry {
    std::string name;
    std::string application_id;
    std::string branch;
    std::string options;
    std::string installation;

    constexpr auto operator==(const FlatpakListEntry&) const -> bool = default;
};

auto parse_apps_from_json(const std::string_view json) -> std::vector<FlatpakListEntry> {
    std::vector<FlatpakListEntry> parsed;
    if (const auto errors = glz::read_json(parsed, json)) {
        // just silencing the nodiscard warning
        // we don't really care why it failed
        // and we'll just return an empty list
    }
    return parsed;
}

auto parse_apps_from_columns(const std::string_view columns) -> std::vector<FlatpakListEntry> {
    using std::views::transform, std::views::split, std::views::filter, std::ranges::to;
    using stringutil::trim, util::filter_transform;
    return columns | split('\n') |
           filter_transform([](auto&& line) -> std::optional<FlatpakListEntry> {
               const auto columns = stringutil::split(trim(std::string_view{line}), '\t');
               if (columns.size() != 5) {
                   return std::nullopt;
               }
               return FlatpakListEntry{
                   .name{columns[0]},
                   .application_id{columns[1]},
                   .branch{columns[2]},
                   .options{columns[3]},
                   .installation{columns[4]},
               };
           }) |
           to<std::vector<FlatpakListEntry>>();
}

auto apps_from_parsed_list(const std::span<const FlatpakListEntry> parsed_list)
    -> std::unordered_map<std::string, FlatpakApp> {
    std::unordered_map<std::string, FlatpakApp> apps;
    for (const auto& ref : parsed_list) {
        auto& app = apps[ref.application_id];

        if (app.name.empty()) {
            app.name = ref.name;
            app.app_id = ref.application_id;
        }

        app.branches.emplace_back(
            ref.branch,
            ref.installation == "system" ? FlatpakApp::Installation::System
                                         : FlatpakApp::Installation::User,
            ref.options.contains("current")
        );
    }
    return apps;
}

} // namespace detail

// Use this one with flatpak 1.17.0 or newer
auto installed_apps_from_json() -> std::unordered_map<std::string, FlatpakApp> {
    const auto result = subprocess::run_command("flatpak", "list", "--app", "--json", columns_arg);
    if (!result || result->exit_code != 0) {
        return {};
    }
    return detail::apps_from_parsed_list(detail::parse_apps_from_json(result->stdout));
}

// Use this one with flatpak older than 1.17.0
auto installed_apps_from_columns() -> std::unordered_map<std::string, FlatpakApp> {
    const auto result = subprocess::run_command("flatpak", "list", "--app", columns_arg);
    if (!result || result->exit_code != 0) {
        return {};
    }
    return detail::apps_from_parsed_list(detail::parse_apps_from_columns(result->stdout));
}

} // namespace flatpakutil
