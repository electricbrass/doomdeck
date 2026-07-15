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

export module directories;

import std;
import appinfo;
import errors;

namespace fs = std::filesystem;

export namespace dirs {

class Directories {
private:
    static constexpr std::string_view app_subdir = appinfo::lowercase_name;
    // static constexpr std::string_view app_subdir = appinfo::app_id;

    // fs::path m_bin_home;
    fs::path m_cache_home;
    fs::path m_config_home;
    fs::path m_data_home;
    fs::path m_state_home;
    // std::optional<fs::path> m_runtime_dir;
    std::vector<fs::path> m_data_dirs;
    // std::vector<fs::path> m_system_config_dirs;
    std::vector<fs::path> m_data_search_dirs; // xdg_data_dirs + m_xdg_data_home

    std::optional<fs::path> m_doomwaddir;
    std::vector<fs::path> m_doomwadpath;

    [[nodiscard]]
    static auto home_dir() -> std::optional<fs::path> {
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

        return std::nullopt;
    }

    [[nodiscard]]
    static auto load_path(const char* env_var) -> std::optional<fs::path> {
        if (const char* env = std::getenv(env_var)) {
            fs::path path{env};
            if (!path.empty() && !path.is_relative()) {
                return path.lexically_normal();
            }
        }

        return std::nullopt;
    }

    [[nodiscard]]
    static auto load_path_list(const char* env_var) -> std::vector<fs::path> {
        if (const char* env = std::getenv(env_var)) {
            std::string_view view{env};
            std::vector<fs::path> dirs;
            for (const auto dir : std::views::split(view, ':')) {
                dirs.emplace_back(std::string_view(dir));
            }
            return dirs;
        }
        return {};
    }

    static void normalize_path_list(std::vector<fs::path>& paths) {
        std::unordered_set<fs::path> seen;
        std::erase_if(paths, [&seen](fs::path& path) {
            if (path.empty() || path.is_relative()) {
                return true;
            }

            path = path.lexically_normal();
            return !seen.insert(path).second;
        });
    }

public:
    Directories() {
        const auto home = home_dir();
        if (!home) {
            throw errors::RuntimeError("User home directory not found");
        }

        if (home->is_relative()) {
            throw errors::RuntimeError("User home directory is relative");
        }

        m_cache_home = load_path("XDG_CACHE_HOME").value_or(*home / ".cache");
        m_config_home = load_path("XDG_CONFIG_HOME").value_or(*home / ".config");
        m_data_home = load_path("XDG_DATA_HOME").value_or(*home / ".local/share");
        m_state_home = load_path("XDG_STATE_HOME").value_or(*home / ".local/state");

        m_data_dirs = load_path_list("XDG_DATA_DIRS");
        normalize_path_list(m_data_dirs);

        if (m_data_dirs.empty()) {
            m_data_dirs = {"/usr/local/share", "/usr/share"};
        }

        m_data_search_dirs.push_back(m_data_home);

        for (const auto& dir : m_data_dirs) {
            m_data_search_dirs.push_back(dir);
        }

        normalize_path_list(m_data_search_dirs);

        m_doomwaddir = load_path("DOOMWADDIR");
        m_doomwadpath = load_path_list("DOOMWADPATH");
        normalize_path_list(m_doomwadpath);
    }

    [[nodiscard]]
    auto cache_home() const -> fs::path {
        return m_cache_home;
    }

    [[nodiscard]]
    auto config_home() const -> fs::path {
        return m_config_home;
    }

    [[nodiscard]]
    auto data_home() const -> fs::path {
        return m_data_home;
    }

    [[nodiscard]]
    auto state_home() const -> fs::path {
        return m_state_home;
    }

    [[nodiscard]]
    auto data_dirs() const -> std::vector<fs::path> {
        return m_data_dirs;
    }

    [[nodiscard]]
    auto data_search_dirs() const -> std::vector<fs::path> {
        return m_data_search_dirs;
    }

    [[nodiscard]]
    auto app_cache() const -> fs::path {
        return cache_home() / app_subdir;
    }

    [[nodiscard]]
    auto app_config() const -> fs::path {
        return config_home() / app_subdir;
    }

    [[nodiscard]]
    auto app_data() const -> fs::path {
        return data_home() / app_subdir;
    }

    [[nodiscard]]
    auto app_state() const -> fs::path {
        return state_home() / app_subdir;
    }

    [[nodiscard]]
    auto doomwaddir() const -> std::optional<fs::path> {
        return m_doomwaddir;
    }

    [[nodiscard]]
    auto doomwadpath() const -> std::vector<fs::path> {
        return m_doomwadpath;
    }
};

} // namespace dirs
