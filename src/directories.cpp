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

export module directories;

import std;
import appinfo;

namespace fs = std::filesystem;

export class XdgDirectories {
private:
    [[nodiscard]]
    // TODO: cstring_view for better
    auto base_dir(std::string_view env_var) -> std::optional<fs::path> {
        fs::path result;
        if (const char* env = std::getenv(env_var.data())) {
            result = fs::path(env);
        } else {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        if (result.is_relative()) {
            return std::nullopt;
        }

        if (fs::exists(result) && !fs::is_directory(result)) {
            return std::nullopt;
        }

        return result.lexically_normal();
    }

public:
};
