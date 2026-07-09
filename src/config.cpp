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

export module config;

import std;

export namespace config {

enum struct ThumbnailType { Titlepic, Endoom };
enum struct ControllerButtons { Steam, Nintendo, Xbox, Playstation };

struct ApplicationSettings {
    bool fullscreen = false;
    bool swap_face_buttons = false;
    ThumbnailType thumbnail_type = ThumbnailType::Titlepic;
    ControllerButtons controller_buttons = ControllerButtons::Steam;
};

struct Config {
    static auto load(std::filesystem::path) -> Config { return {}; }
    void save(std::filesystem::path) {}
    ApplicationSettings settings;
};

} // namespace config
