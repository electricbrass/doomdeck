export module stringutils;

import std;

export namespace stringutil {

[[nodiscard]]
inline auto ltrim(std::string_view s) -> std::string_view {
    const auto it = std::ranges::find_if(s, [](unsigned char c) { return !std::isspace(c); });
    s.remove_prefix(static_cast<std::string_view::size_type>(it - s.begin()));
    return s;
}

[[nodiscard]]
inline auto rtrim(std::string_view s) -> std::string_view {
    // TODO: verify that using reverse like this is safe
    const auto it = std::ranges::find_if(std::views::reverse(s),
                                         [](unsigned char c) { return !std::isspace(c); });
    s.remove_suffix(static_cast<std::string_view::size_type>(it - s.rbegin()));
    return s;
}

[[nodiscard]]
inline auto trim(const std::string_view s) -> std::string_view {
    return ltrim(rtrim(s));
}

} // namespace stringutil
