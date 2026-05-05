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

export module desktopfileloader;

import std;

namespace fs = std::filesystem;

// TODO: maybe restucture to have static factory methods
// that are use for different ways of loading the paths
// e.g. from an argument, from environment variables
// and then they can return a small struct containing
// warnings to be logged, about things like invalid paths
// probably need to disallow any relative paths
// maybe even create another class that just handles
// loading paths from environment variables and checking
// stuff like being absolute, uniqueness, normalizing, etc
// cause that will also be needed for DOOMWADDIR/DOOMWADPATH
// ...but then how will that work with fallbacks?
// might need to store multiple layers of directories
// so that we can have overrides work properly? or maybe just make sure to
// put things in the right order, but then thats less explicit

namespace {

inline auto ltrim(std::string_view s) -> std::string_view {
    const auto it = std::ranges::find_if(s, [](unsigned char c) { return !std::isspace(c); });
    s.remove_prefix(static_cast<std::string_view::size_type>(it - s.begin()));
    return s;
}

inline auto rtrim(std::string_view s) -> std::string_view {
    // TODO: verify that using reverse like this is safe
    const auto it = std::ranges::find_if(std::views::reverse(s),
                                         [](unsigned char c) { return !std::isspace(c); });
    s.remove_suffix(static_cast<std::string_view::size_type>(it - s.rbegin()));
    return s;
}

inline auto trim(const std::string_view s) -> std::string_view { return ltrim(rtrim(s)); }

} // namespace

export class DesktopFileLoader {
private:
    std::vector<fs::path> m_applicationDirs;

    struct DesktopFile {
        // should ignore all with type != application
        std::string name;
        std::optional<bool> exec;
        std::vector<std::string> keywords;
        std::vector<std::string> categories;
        std::vector<std::string> mimetypes;
        std::optional<bool> hidden;
    };
    std::unordered_map<std::string, DesktopFile> m_parsedFiles;

    enum struct ParseError {};

    struct SectionHeader {
        std::string name;
    };

    struct Entry {
        std::string key;
        std::string value;
    };

    using DesktopFileLine = std::variant<SectionHeader, Entry>;

    // TODO: just make this take in a stream or something, so that parsing can be
    // tested from just strings
    static auto parseFile(std::istream& stream) -> std::expected<DesktopFile, ParseError> {
        using enum ParseError;
        DesktopFile file;
        std::string line;
        while (std::getline(stream, line)) {
            std::string_view trimmed = trim(line);
            if (trimmed.empty() || trimmed[0] == '#') {
                continue;
            }
            if (trimmed.front() == '[' && trimmed.back() == ']') {
                // section header
            }
        }
        return file;
    }

    void normalizePaths() {
        // normalize and deduplicate
        std::unordered_set<fs::path> seen;
        std::erase_if(m_applicationDirs, [&seen](fs::path& dir) {
            if (dir.is_relative()) {
                return true;
            }
            dir = (dir / "applications").lexically_normal();
            return !seen.insert(dir).second;
        });
    }

public:
    DesktopFileLoader() {
        const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
        const char* xdgDataDirs = std::getenv("XDG_DATA_DIRS");

        if (xdgDataHome) {
            m_applicationDirs.emplace_back(xdgDataHome);
        } else {
            const char* home = std::getenv("HOME");
            // TODO: should this be an exception instead?
            // If so, this should be restructured to be a
            // static method instead of a constructor
            if (home) {
                m_applicationDirs.emplace_back(fs::path(home) / ".local/share");
            }
        }

        if (xdgDataDirs) {
            std::string_view view(xdgDataDirs);
            for (const auto dir : std::views::split(view, ':')) {
                m_applicationDirs.emplace_back(std::string_view(dir));
            }
        } else {
            m_applicationDirs.emplace_back("/usr/local/share");
            m_applicationDirs.emplace_back("/usr/share");
        }

        normalizePaths();
    }

    void load() {
        for (const auto& dir : m_applicationDirs) {
            if (!fs::is_directory(dir)) {
                // TODO: log error somehow? or just filter?
                continue;
            }

            for (const auto& entry : std::views::filter(
                     fs::directory_iterator(dir), [](const fs::directory_entry& entry) {
                         return entry.path().extension() == ".desktop";
                     })) {
                std::ifstream stream(entry.path());
                if (!stream) {
                    // log error somehow
                    continue;
                }

                // Only the first entry of a particular name found should be used
                if (m_parsedFiles.contains(entry.path().stem().string())) {
                    continue;
                }

                if (const auto result = parseFile(stream)) {
                    m_parsedFiles[entry.path().stem().string()] = *result;
                } else {
                    // log error somehow
                }
            }
        }
    }
};
