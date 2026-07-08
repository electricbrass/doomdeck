export module stringutils;

import std;

export namespace stringutil {

[[nodiscard]]
constexpr auto is_ascii_whitespace(const unsigned char c) -> bool {
    switch (c) {
        case ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
            return true;
        default:
            return false;
    }
}

[[nodiscard]]
constexpr auto is_ascii_uppercase(const unsigned char c) -> bool {
    return c >= 'A' && c <= 'Z';
}

[[nodiscard]]
constexpr auto is_ascii_lowercase(const unsigned char c) -> bool {
    return c >= 'a' && c <= 'z';
}

[[nodiscard]]
constexpr auto ltrim(std::string_view s) -> std::string_view {
    const auto it = std::ranges::find_if_not(s, is_ascii_whitespace);
    s.remove_prefix(static_cast<std::string_view::size_type>(it - s.begin()));
    return s;
}

[[nodiscard]]
constexpr auto rtrim(std::string_view s) -> std::string_view {
    // TODO: verify that using reverse like this is safe
    const auto it = std::ranges::find_if_not(std::views::reverse(s), is_ascii_whitespace);
    s.remove_suffix(static_cast<std::string_view::size_type>(it - s.rbegin()));
    return s;
}

[[nodiscard]]
constexpr auto trim(const std::string_view s) -> std::string_view {
    return ltrim(rtrim(s));
}

[[nodiscard]]
constexpr auto to_ascii_uppercase(const char c) -> char {
    // clang-format off
    return is_ascii_lowercase(static_cast<unsigned char>(c))
        ? static_cast<char>(c + ('a' - 'A')) : c;
    // clang-format on
}

[[nodiscard]]
constexpr auto to_ascii_lowercase(const char c) -> char {
    // clang-format off
    return is_ascii_uppercase(static_cast<unsigned char>(c))
        ? static_cast<char>(c + ('a' - 'A')) : c;
    // clang-format on
}

[[nodiscard]]
constexpr auto to_ascii_uppercase(const std::string_view s) -> std::string {
    std::string result{s};
    for (auto& c : result) {
        c = to_ascii_uppercase(c);
    }
    return result;
}

[[nodiscard]]
constexpr auto to_ascii_lowercase(const std::string_view s) -> std::string {
    std::string result{s};
    for (auto& c : result) {
        c = to_ascii_lowercase(c);
    }
    return result;
}

[[nodiscard]]
constexpr auto make_ascii_uppercase(std::string& s) -> std::string& {
    for (auto& c : s) {
        c = to_ascii_uppercase(c);
    }
    return s;
}

[[nodiscard]]
constexpr auto make_ascii_lowercase(std::string& s) -> std::string& {
    for (auto& c : s) {
        c = to_ascii_lowercase(c);
    }
    return s;
}

} // namespace stringutil
