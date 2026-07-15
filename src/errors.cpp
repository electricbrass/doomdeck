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

export module errors;

import std;

export namespace errors {

template <typename EXCEPTION>
class ErrorWithStacktrace : public EXCEPTION {
private:
    std::stacktrace m_stacktrace;

public:
    template <typename... ARGS>
    explicit ErrorWithStacktrace(std::format_string<ARGS...> fmt, ARGS&&... args)
        : EXCEPTION(std::format(fmt, std::forward<ARGS>(args)...)),
          m_stacktrace(std::stacktrace::current(1)) {}

    explicit ErrorWithStacktrace(const std::string& message)
        : EXCEPTION(message), m_stacktrace(std::stacktrace::current(1)) {}

    explicit ErrorWithStacktrace(const char* message)
        : EXCEPTION(message), m_stacktrace(std::stacktrace::current(1)) {}

    [[nodiscard]]
    auto stacktrace() const noexcept -> const std::stacktrace& {
        return m_stacktrace;
    }
};

using RuntimeError = ErrorWithStacktrace<std::runtime_error>;
using LogicError = ErrorWithStacktrace<std::logic_error>;

} // namespace errors
