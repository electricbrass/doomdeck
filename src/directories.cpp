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

class XdgDirectories {
private:
    // fs::path m_bin_home;
    fs::path m_cache_home;
    fs::path m_config_home;
    fs::path m_data_home;
    fs::path m_state_home;
    // std::optional<fs::path> m_runtime_dir;
    std::vector<fs::path> m_system_data_dirs; // xdg_data_dirs
    // std::vector<fs::path> m_system_config_dirs;
    std::vector<fs::path> m_data_dirs; // xdg_data_dirs + m_xdg_data_home

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
    static auto base_dir(const char* env_var) -> std::optional<fs::path> {
        fs::path result;
        if (const char* env = std::getenv(env_var)) {
            result = fs::path(env);
        } else {
            return std::nullopt;
        }

        if (result.empty() || result.is_relative()) {
            return std::nullopt;
        }

        return result.lexically_normal();
    }

public:
    XdgDirectories() {
        const auto home = home_dir();
        if (!home) {
            throw errors::ApplicationError("User home directory not found");
        }

        if (home->is_relative()) {
            throw errors::ApplicationError("User home directory is relative");
        }

        m_cache_home = base_dir("XDG_CACHE_HOME").value_or(*home / ".cache");
        m_config_home = base_dir("XDG_CONFIG_HOME").value_or(*home / ".config");
        m_data_home = base_dir("XDG_DATA_HOME").value_or(*home / ".local/share");
        m_state_home = base_dir("XDG_STATE_HOME").value_or(*home / ".local/state");

        if (const char* dataDirs = std::getenv("XDG_DATA_DIRS")) {
            std::string_view view{dataDirs};
            for (const auto dir : std::views::split(view, ':')) {
                m_system_data_dirs.emplace_back(std::string_view(dir));
            }
        }

        std::unordered_set<fs::path> seen;
        std::erase_if(m_system_data_dirs, [&seen](fs::path& dir) {
            if (dir.is_relative() || dir.empty()) {
                return true;
            }
            dir = dir.lexically_normal();
            return !seen.insert(dir).second;
        });

        if (m_system_data_dirs.empty()) {
            m_system_data_dirs = {"/usr/local/share", "/usr/share"};
        }

        m_data_dirs.push_back(m_data_home);

        for (const auto& dir : m_system_data_dirs) {
            m_data_dirs.push_back(dir);
        }

        seen.clear();
        std::erase_if(m_data_dirs,
                      [&seen](const fs::path& dir) { return !seen.insert(dir).second; });
    }

    [[nodiscard]]
    auto cache_home() const -> const fs::path& {
        return m_cache_home;
    }

    [[nodiscard]]
    auto config_home() const -> const fs::path& {
        return m_config_home;
    }

    [[nodiscard]]
    auto data_home() const -> const fs::path& {
        return m_data_home;
    }

    [[nodiscard]]
    auto state_home() const -> const fs::path& {
        return m_state_home;
    }

    [[nodiscard]]
    // TODO: should this be std::span<const fs::path> instead?
    auto system_data_dirs() const -> const std::vector<fs::path>& {
        return m_system_data_dirs;
    }

    [[nodiscard]]
    // TODO: should this be std::span<const fs::path> instead?
    auto data_dirs() const -> const std::vector<fs::path>& {
        return m_data_dirs;
    }
};

[[nodiscard]]
auto app_cache(const XdgDirectories& dirs) -> fs::path {
    return dirs.cache_home() / appinfo::lowercase_name;
}

[[nodiscard]]
auto app_config(const XdgDirectories& dirs) -> fs::path {
    return dirs.config_home() / appinfo::lowercase_name;
}

[[nodiscard]]
auto app_data(const XdgDirectories& dirs) -> fs::path {
    return dirs.data_home() / appinfo::lowercase_name;
}

[[nodiscard]]
auto app_state(const XdgDirectories& dirs) -> fs::path {
    return dirs.state_home() / appinfo::lowercase_name;
}

[[nodiscard]]
auto doomwaddir() -> std::optional<fs::path> {
    if (const char* waddir = std::getenv("DOOMWADDIR")) {
        if (*waddir != '\0') {
            const fs::path path{waddir};
            if (!path.is_relative()) {
                return path.lexically_normal();
            }
        }
    }
    return std::nullopt;
}

[[nodiscard]]
auto doomwadpath() -> std::vector<fs::path> {
    if (const char* wadpath = std::getenv("DOOMWADPATH")) {
        std::string_view view{wadpath};
        std::vector<fs::path> dirs;
        for (const auto dir : std::views::split(view, ':')) {
            dirs.emplace_back(std::string_view(dir));
        }

        std::unordered_set<fs::path> seen;
        std::erase_if(dirs, [&seen](fs::path& dir) {
            if (dir.is_relative() || dir.empty()) {
                return true;
            }
            dir = dir.lexically_normal();
            return !seen.insert(dir).second;
        });

        return dirs;
    }
    return {};
}

} // namespace dirs
