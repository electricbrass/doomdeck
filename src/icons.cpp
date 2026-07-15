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
    struct UVCoords {
        float x0, y0, x1, y1;
    };
    std::span<const std::uint8_t> png;
    int width, height;
    std::array<UVCoords, static_cast<std::size_t>(ControllerButton::FaceRight) + 1> coords;
    constexpr auto operator[](ControllerButton button) const -> UVCoords {
        return coords.at(static_cast<std::size_t>(button));
    }
};

namespace {

struct Coords {
    int x, y;
};

// TODO: maybe move this to Atlas constructor
// would reduce duplication of width and height
// also it doesn't really need to be templated,
// the length is always the number of ControllerButton variants
template <std::size_t N>
consteval auto make_coords(const int width, const int height, const std::array<Coords, N>& coords)
    -> std::array<Atlas::UVCoords, N> {
    std::array<Atlas::UVCoords, N> result{};

    for (auto&& [out, in] : std::views::zip(result, coords)) {
        const auto [x, y] = in;

        out = {
            .x0 = static_cast<float>(x) / static_cast<float>(width),
            .y0 = static_cast<float>(height - y - Atlas::icon_size) / static_cast<float>(height),
            .x1 = static_cast<float>(x + Atlas::icon_size) / static_cast<float>(width),
            .y1 = static_cast<float>(height - y) / static_cast<float>(height),
        };
    }

    return result;
}

} // namespace

// clang-format off
export constexpr Atlas steam_atlas{
    .png = steam_png,
    .width = 1408,
    .height = 1408,
    .coords = make_coords(1408, 1408, std::to_array<Coords>({
        {.x = 896,  .y = 0  }, // L1
        {.x = 1024, .y = 128}, // R1
        {.x = 1152, .y = 0  }, // L2
        {.x = 1280, .y = 128}, // R2
        {.x = 1152, .y = 256}, // Y
        {.x = 128,  .y = 0  }, // A
        {.x = 896,  .y = 256}, // X
        {.x = 384,  .y = 0  }, // B
    }))
};

export constexpr Atlas nintendo_atlas{
    .png = nintendo_png,
    .width = 1536,
    .height = 1408,
    .coords = make_coords(1536, 1408, std::to_array<Coords>({
        {.x = 512,  .y = 128}, // L
        {.x = 1280, .y = 128}, // R
        {.x = 1280, .y = 256}, // ZL
        {.x = 384,  .y = 128}, // ZR
        {.x = 768,  .y = 256}, // X
        {.x = 768,  .y = 0  }, // B
        {.x = 1024, .y = 256}, // Y
        {.x = 512,  .y = 0  }, // A
    }))
};

export constexpr Atlas playstation_atlas{
    .png = playstation_png,
    .width = 1536,
    .height = 1536,
    .coords = make_coords(1536, 1536, std::to_array<Coords>({
        {.x = 384,  .y = 640}, // L1
        {.x = 1408, .y = 640}, // R1
        {.x = 896,  .y = 640}, // L2
        {.x = 384,  .y = 768}, // R2
        {.x = 384,  .y = 128}, // Triangle
        {.x = 1408, .y = 0  }, // Cross
        {.x = 128,  .y = 128}, // Square
        {.x = 1152, .y = 0  }, // Circle
    }))
};

export constexpr Atlas xbox_atlas{
    .png = xbox_png,
    .width = 1280,
    .height = 1280,
    .coords = make_coords(1280, 1280, std::to_array<Coords>({
        {.x = 896,  .y = 768}, // LB
        {.x = 384,  .y = 896}, // RB
        {.x = 128,  .y = 896}, // LT
        {.x = 896,  .y = 896}, // RT
        {.x = 1024, .y = 128}, // Y
        {.x = 256,  .y = 128}, // A
        {.x = 768,  .y = 128}, // X
        {.x = 512,  .y = 128}, // B
    }))
};
// clang-format on

} // namespace icons
