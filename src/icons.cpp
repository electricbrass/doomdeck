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

export module icons;

import std;

// GCC also supports #embed in all C/C++ versions, and we don't support MSVC
#pragma clang diagnostic ignored "-Wc23-extensions"

namespace icons {

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
constexpr std::uint8_t steam_png[] = {
#embed "kenney_input_prompts/steam/steam-deck_sheet_double.png"
};

constexpr std::uint8_t nintendo_png[] = {
#embed "kenney_input_prompts/nintendo/nintendo-switch-2_sheet_double.png"
};

constexpr std::uint8_t playstation_png[] = {
#embed "kenney_input_prompts/playstation/playstation-series_sheet_double.png"
};

constexpr std::uint8_t xbox_png[] = {
#embed "kenney_input_prompts/xbox/xbox-series_sheet_double.png"
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)

} // namespace

// TODO: add Auto type
export enum struct ControllerType { Steam, Nintendo, Xbox, Playstation };

// TODO: add conversions from imgui and SDL enums
export enum struct ControllerButton {
    LeftShoulder,
    RightShoulder,
    LeftTrigger,
    RightTrigger,
    FaceTop,
    FaceBottom,
    FaceLeft,
    FaceRight,
};

// TODO: add outline and fill options
// current coordinates are for fill only
export struct Atlas {
    static constexpr int icon_size = 128;
    struct Coords {
        int x, y;
    };
    std::span<const std::uint8_t> png;
    int width, height;
    std::array<Coords, static_cast<std::size_t>(ControllerButton::FaceRight) + 1> coords;
    constexpr auto operator[](ControllerButton button) const -> Coords {
        return coords.at(static_cast<std::size_t>(button));
    }
};

// clang-format off
export constexpr Atlas steam_atlas{
    .png = steam_png,
    .width = 1408,
    .height = 1408,
    .coords = {{
        {.x = 896,  .y = 0  }, // L1
        {.x = 1024, .y = 128}, // R1
        {.x = 1152, .y = 0  }, // L2
        {.x = 1280, .y = 128}, // R2
        {.x = 1152, .y = 256}, // Y
        {.x = 128,  .y = 0  }, // A
        {.x = 896,  .y = 256}, // X
        {.x = 384,  .y = 0  }, // B
    }}
};

export constexpr Atlas nintendo_atlas{
    .png = nintendo_png,
    .width = 1536,
    .height = 1408,
    .coords = {{
        {.x = 512,  .y = 128}, // L
        {.x = 1280, .y = 128}, // R
        {.x = 1280, .y = 256}, // ZL
        {.x = 384,  .y = 128}, // ZR
        {.x = 768,  .y = 256}, // X
        {.x = 768,  .y = 0  }, // B
        {.x = 1024, .y = 256}, // Y
        {.x = 512,  .y = 0  }, // A
    }}
};

export constexpr Atlas playstation_atlas{
    .png = playstation_png,
    .width = 1536,
    .height = 1536,
    .coords = {{
        {.x = 384,  .y = 640}, // L1
        {.x = 1408, .y = 640}, // R1
        {.x = 896,  .y = 640}, // L2
        {.x = 384,  .y = 768}, // R2
        {.x = 384,  .y = 128}, // Triangle
        {.x = 1408, .y = 0  }, // Cross
        {.x = 128,  .y = 128}, // Square
        {.x = 1152, .y = 0  }, // Circle
    }}
};

export constexpr Atlas xbox_atlas{
    .png = xbox_png,
    .width = 1280,
    .height = 1280,
    .coords = {{
        {.x = 896,  .y = 768}, // LB
        {.x = 384,  .y = 896}, // RB
        {.x = 128,  .y = 896}, // LT
        {.x = 896,  .y = 896}, // RT
        {.x = 1024, .y = 128}, // Y
        {.x = 256,  .y = 128}, // A
        {.x = 768,  .y = 128}, // X
        {.x = 512,  .y = 128}, // B
    }}
};
// clang-format on

} // namespace icons
