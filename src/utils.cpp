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
import subprocess;

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

template <typename T>
concept OptionalLike = requires(T x) {
    { x.has_value() } -> std::convertible_to<bool>;
    *x;
};

template <typename F>
struct FilterTransformClosure : std::ranges::range_adaptor_closure<FilterTransformClosure<F>> {
    F func;

    template <std::ranges::viewable_range R>
        requires OptionalLike<std::invoke_result_t<F&, std::ranges::range_reference_t<R>>>
    constexpr auto operator()(R&& range) const {
        return std::forward<R>(range) | std::views::transform(func) |
               std::views::filter([](const auto& x) { return x.has_value(); }) |
               std::views::transform([](auto&& x) { return *x; });
    }
};

template <typename F>
constexpr auto filter_transform(F&& func) {
    return FilterTransformClosure<std::decay_t<F>>{
        .func = std::forward<F>(func),
    };
}

enum struct OpenFileError {
    XdgOpenNotFound,
    FileNotFound,
    RelativePath,
    Other,
};

auto open_file(const std::filesystem::path& path) -> std::expected<void, OpenFileError> {
    using enum OpenFileError;
    if (path.is_relative()) {
        return std::unexpected{RelativePath};
    }

    if (const auto result = subprocess::run_command("xdg-open", path)) {
        switch (result->exit_code) {
            // from xdg-open man page
            case 0:
                return {};
            case 2:
                return std::unexpected{FileNotFound};
            default:
                return std::unexpected{Other};
        }
    } else {
        if (result.error() == subprocess::SubprocessError::NotFound) {
            return std::unexpected{XdgOpenNotFound};
        }
        return std::unexpected{Other};
    };
}

} // namespace util
