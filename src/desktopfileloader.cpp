export module desktopfileloader;

import std;

namespace fs = std::filesystem;

export class DesktopFileLoader {
private:
    std::vector<fs::path> m_applicationDirs;

public:
    DesktopFileLoader() {
        const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
        const char* xdgDataDirs = std::getenv("XDG_DATA_DIRS");

        if (xdgDataHome) {
            m_applicationDirs.emplace_back(xdgDataHome);
        } else {
            const char* home = std::getenv("HOME");
            // TODO: should this be an exception instead?
            // If so, this should be restructured to be a
            // static method instead of a constructor
            if (home) {
                m_applicationDirs.emplace_back(fs::path(home) / ".local/share");
            }
        }

        if (xdgDataDirs) {
            std::string_view view(xdgDataDirs);
            for (const auto dir : std::views::split(view, ':')) {
                m_applicationDirs.emplace_back(std::string_view(dir));
            }
        } else {
            m_applicationDirs.emplace_back("/usr/share");
            m_applicationDirs.emplace_back("/usr/local/share");
        }

        // deduplicate
        std::ranges::sort(m_applicationDirs);
        const auto [first, last] = std::ranges::unique(m_applicationDirs);
        m_applicationDirs.erase(first, last);

        std::ranges::for_each(m_applicationDirs, [](fs::path& dir) {
            dir = (dir / "applications").lexically_normal();
        });
    }

    void load() {
        for (const auto& dir : m_applicationDirs) {
        }
    }
};
