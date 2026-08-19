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
import stringutils;
import directories;

namespace fs = std::filesystem;
using namespace std::string_view_literals;

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

// constexpr std::array doomMimeTypes = {
//     "application/x-doom-wad"sv,
//     "application/x-doom-pk3"sv,
//     "application/x-doom-pk7"sv,
//     "application/x-doom-pke"sv,
// };

// constexpr std::array doomCategories = {
//     "Doom"sv, "Heretic"sv, "Hexen"sv, "Shooter"sv, "Game"sv, "ActionGame"sv,
// };

// constexpr std::array doomKeywords = {
//     "Doom"sv, "Heretic"sv, "Hexen"sv, "iwad"sv,   "pwad"sv,
//     "boom"sv, "mbf"sv,     "mbf21"sv, "prboom"sv, "zdoom"sv,
// };

} // namespace

export class DesktopFileLoader {
private:
    std::vector<fs::path> m_application_dirs;

    struct DesktopFile {
        // should ignore all with type != application
        std::string name;
        std::optional<bool> exec;
        std::optional<std::string> icon; // may or may not use this
        std::vector<std::string> keywords;
        std::vector<std::string> categories;
        std::vector<std::string> mimetypes;
        std::optional<bool> hidden;
        std::optional<bool> terminal;       // might need this? not sure yet
        std::optional<std::string> flatpak; // flatpak id from X-Flatpak
        fs::path path;
    };
    std::unordered_map<std::string, DesktopFile> m_parsedFiles;

    enum struct ParseError {
        MissingHeader,
    };

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
    static auto parse_file(std::istream& stream, fs::path path = {})
        -> std::expected<DesktopFile, ParseError> {
        using stringutil::trim;
        using enum ParseError;
        DesktopFile file;
        file.path = std::move(path);
        std::string line;
        enum struct ParseState {
            Header,
            Section,
        };
        ParseState state = ParseState::Header;
        while (std::getline(stream, line)) {
            std::string_view trimmed = trim(line);
            if (trimmed.empty() || trimmed[0] == '#') {
                continue;
            }
            switch (state) {
                using enum ParseState;
                case Header:
                    if (trimmed.front() == '[' && trimmed.back() == ']') {
                        trimmed.remove_prefix(1);
                        trimmed.remove_suffix(1);
                        if (trim(trimmed) != "Desktop Entry"sv) {
                            return std::unexpected(MissingHeader);
                        }
                        state = Section;
                        continue;
                    }
                    break;
                case Section:

                    break;
            }
        }
        return file;
    }

    [[nodiscard]]
    static auto desktop_file_id(const fs::path& base, const fs::path& path) -> std::string {
        const fs::path relative = path.lexically_relative(base).replace_extension();
        std::string result = relative.string();
        std::ranges::replace(result, '/', '-');
        return result;
    }

public:
    DesktopFileLoader() : m_application_dirs(dirs::Directories().data_search_dirs()) {}

    void load() {
        for (const auto& dir : m_application_dirs) {
            if (!fs::is_directory(dir)) {
                // TODO: log error somehow? or just filter?
                continue;
            }

            // TODO: scan subdirectories as well
            for (const auto& entry : std::views::filter(
                     fs::directory_iterator(dir), [](const fs::directory_entry& entry) {
                         return entry.path().extension() == ".desktop";
                     }
                 )) {
                std::ifstream stream(entry.path());
                if (!stream) {
                    // log error somehow
                    continue;
                }

                // Only the first entry of a particular name found should be used
                if (m_parsedFiles.contains(entry.path().stem().string())) {
                    continue;
                }

                if (const auto result = parse_file(stream, entry.path())) {
                    m_parsedFiles[desktop_file_id(dir, entry.path())] = *result;
                } else {
                    // log error somehow
                }
            }
        }
    }
};
