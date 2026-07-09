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

export module ui.types;

import std;

export namespace ui {

// TODO: should this go in appstate instead?
enum struct Tab { Launcher, Pwads, Iwads, Ports, Settings };

constexpr auto next_tab(Tab current) -> Tab {
    return static_cast<Tab>(
        std::min(std::to_underlying(current) + 1, std::to_underlying(Tab::Settings)));
}

constexpr auto prev_tab(Tab current) -> Tab {
    return static_cast<Tab>(std::max(std::to_underlying(current) - 1, 0));
}

} // namespace ui
