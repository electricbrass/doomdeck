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

export module utils;

import std;

export namespace util {

struct Version {
    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;

    constexpr auto operator<=>(const Version&) const = default;
};

template <typename... TYPES>
struct Visitor : TYPES... {
    using TYPES::operator()...;
};

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>
constexpr auto parse_num(const std::string_view str) -> std::optional<T> {
    T value{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (ec != std::errc{} || ptr != str.data() + str.size()) {
        return std::nullopt;
    }

    return value;
}

} // namespace util
