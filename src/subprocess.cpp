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

module;

#include <cerrno>
#include <experimental/scope>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

export module subprocess;

import std;

namespace subprocess {

export enum struct SubprocessError {
    EmptyArgs,
    Timeout,
    SystemError,
};

export struct CommandResult {
    std::string stdout;
    std::string stderr;
    int exit_code;
};

namespace {

auto make_posix_spawn_args(const std::span<const std::string> args) -> std::vector<char*> {
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& arg : args) {
        // posix_spawn is specified to not modify the args, but
        // for legacy compatiblity reasons it takes a char * const *
        // rather than char const * const *
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        argv.push_back(const_cast<char*>(arg.data()));
    }
    argv.push_back(nullptr);
    return argv;
}

class FileDescriptor {
private:
    int m_fd = -1;

public:
    FileDescriptor() = default;
    explicit FileDescriptor(const int fd) : m_fd(fd) {}
    ~FileDescriptor() { this->close(); }

    FileDescriptor(const FileDescriptor&) = delete;
    auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;
    FileDescriptor(FileDescriptor&& other) noexcept : m_fd(other.m_fd) { other.m_fd = -1; }
    auto operator=(FileDescriptor&& other) noexcept -> FileDescriptor& {
        if (this != &other) {
            this->close();
            m_fd = other.release();
        }
        return *this;
    }

    [[nodiscard]] explicit operator bool() const { return m_fd != -1; }
    [[nodiscard]] auto get() const -> int { return m_fd; }

    auto release() -> int { return std::exchange(m_fd, -1); }

    void close() {
        if (m_fd != -1) {
            ::close(m_fd);
        }

        m_fd = -1;
    }
};

struct Pipe {
    FileDescriptor read_fd;
    FileDescriptor write_fd;
};

auto create_pipe() -> std::optional<Pipe> {
    std::array<int, 2> pipe_array{};
    if (pipe(pipe_array.data()) == -1) {
        return std::nullopt;
    }
    return Pipe{
        .read_fd = FileDescriptor{pipe_array[0]}, .write_fd = FileDescriptor{pipe_array[1]}
    };
}

} // namespace

export auto run_command(const std::span<const std::string> args)
    -> std::expected<CommandResult, SubprocessError> {
    using std::experimental::scope_exit;
    using enum SubprocessError;

    if (args.empty()) {
        return std::unexpected{EmptyArgs};
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    const scope_exit destroy_actions([&] { posix_spawn_file_actions_destroy(&actions); });

    const FileDescriptor devnull{open("/dev/null", O_RDWR)};

    if (!devnull) {
        return std::unexpected{SystemError};
    }

    auto stdout_pipe = create_pipe();
    if (!stdout_pipe) {
        return std::unexpected{SystemError};
    }

    auto stderr_pipe = create_pipe();
    if (!stderr_pipe) {
        return std::unexpected{SystemError};
    }

    posix_spawn_file_actions_adddup2(&actions, devnull.get(), STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, stdout_pipe->write_fd.get(), STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, stderr_pipe->write_fd.get(), STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, devnull.get());
    posix_spawn_file_actions_addclose(&actions, stdout_pipe->read_fd.get());
    posix_spawn_file_actions_addclose(&actions, stderr_pipe->read_fd.get());

    const std::vector<char*> argv = make_posix_spawn_args(args);

    pid_t pid{};
    if (posix_spawnp(&pid, args[0].c_str(), &actions, nullptr, argv.data(), environ)) {
        // All the errors are pretty low level system errors where the exact reason for failure
        // just doesn't really matter to us other than that it failed.
        return std::unexpected{SystemError};
    }

    stdout_pipe->write_fd.close();
    stderr_pipe->write_fd.close();

    std::string stdout;
    std::string stderr;
    std::array<char, 4096> buffer{};

    std::array<pollfd, 2> fds{
        pollfd{.fd = stdout_pipe->read_fd.get(), .events = POLLIN, .revents = 0},
        pollfd{.fd = stderr_pipe->read_fd.get(), .events = POLLIN, .revents = 0},
    };

    const auto read_pipe = [&](pollfd& pfd, std::string& output,
                               FileDescriptor& fd) -> std::expected<void, SubprocessError> {
        if (!fd) {
            return {};
        }

        if (!(pfd.revents & (POLLIN | POLLHUP))) {
            return {};
        }

        const ssize_t count = read(pfd.fd, buffer.data(), buffer.size());

        if (count > 0) {
            output.append(buffer.data(), static_cast<std::size_t>(count));
        } else if (count == 0) {
            fd.close();
        } else if (errno != EINTR) {
            return std::unexpected{SystemError};
        }

        return {};
    };

    static constexpr int timeout_ms = 5000;

    while (stdout_pipe->read_fd || stderr_pipe->read_fd) {
        const int poll_result = poll(fds.data(), fds.size(), timeout_ms);
        if (poll_result == 0) {
            // timeout
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
            return std::unexpected{Timeout};
        }
        if (poll_result == -1) {
            if (errno == EINTR) {
                continue;
            }

            return std::unexpected(SystemError);
        }

        if (const auto result = read_pipe(fds[0], stdout, stdout_pipe->read_fd); !result) {
            return std::unexpected{result.error()};
        }

        if (const auto result = read_pipe(fds[1], stderr, stderr_pipe->read_fd); !result) {
            return std::unexpected{result.error()};
        }
    }

    int status{};
    const pid_t wait_result = waitpid(pid, &status, 0);
    if (wait_result == -1 || !WIFEXITED(status)) {
        return std::unexpected{SystemError};
    }

    return std::expected<CommandResult, SubprocessError>{
        {.stdout = stdout, .stderr = stderr, .exit_code = WEXITSTATUS(status)}
    };
}

export template <typename... ARGS>
    requires(std::convertible_to<ARGS, std::string> && ...)
auto run_command(ARGS&&... args) -> std::expected<CommandResult, SubprocessError> {
    std::array<std::string, sizeof...(ARGS)> argv{std::string(std::forward<ARGS>(args))...};
    return run_command(argv);
}

// TODO: make redirect_io be controlled by a cli option
// no need to be in settings since it doesn't matter except
// when doomdeck was started from the command line
export auto launch_game(const std::span<const std::string> args, const bool redirect_io = false)
    -> std::expected<void, SubprocessError> {
    using std::experimental::scope_exit;
    using enum SubprocessError;

    if (args.empty()) {
        return std::unexpected{EmptyArgs};
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    const scope_exit destroy_actions([&] { posix_spawn_file_actions_destroy(&actions); });
    FileDescriptor devnull{};
    if (redirect_io) {
        devnull = FileDescriptor{open("/dev/null", O_RDWR)};

        if (!devnull) {
            return std::unexpected{SystemError};
        }

        posix_spawn_file_actions_adddup2(&actions, devnull.get(), STDIN_FILENO);
        posix_spawn_file_actions_adddup2(&actions, devnull.get(), STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, devnull.get(), STDERR_FILENO);
        posix_spawn_file_actions_addclose(&actions, devnull.get());
    }

    const std::vector<char*> argv = make_posix_spawn_args(args);

    pid_t pid{};
    if (posix_spawnp(&pid, args[0].c_str(), &actions, nullptr, argv.data(), environ)) {
        // All the errors are pretty low level system errors where the exact reason for failure
        // just doesn't really matter to us other than that it failed.
        return std::unexpected{SystemError};
    }

    return {};
}

} // namespace subprocess
