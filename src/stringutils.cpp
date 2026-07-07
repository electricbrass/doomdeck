export module stringutils;

import std;

export namespace stringutil {

[[nodiscard]]
constexpr auto is_ascii_space(const unsigned char c) -> bool {
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
constexpr auto ltrim(std::string_view s) -> std::string_view {
    const auto it = std::ranges::find_if_not(s, is_ascii_space);
    s.remove_prefix(static_cast<std::string_view::size_type>(it - s.begin()));
    return s;
}

[[nodiscard]]
constexpr auto rtrim(std::string_view s) -> std::string_view {
    // TODO: verify that using reverse like this is safe
    const auto it = std::ranges::find_if_not(std::views::reverse(s), is_ascii_space);
    s.remove_suffix(static_cast<std::string_view::size_type>(it - s.rbegin()));
    return s;
}

[[nodiscard]]
constexpr auto trim(const std::string_view s) -> std::string_view {
    return ltrim(rtrim(s));
}

} // namespace stringutil
