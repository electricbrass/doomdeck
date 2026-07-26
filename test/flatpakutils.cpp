#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_tostring.hpp>

import std;
import flatpakutils;

using flatpakutil::FlatpakApp;
using flatpakutil::detail::FlatpakListEntry;

template <>
struct Catch::StringMaker<FlatpakListEntry> {
    static std::string convert(const FlatpakListEntry& ref) {
        return std::format(
            "{{name={}, application_id={}, branch={}, options={}, installation={}}}", ref.name,
            ref.application_id, ref.branch, ref.options, ref.installation
        );
    }
};

template <>
struct Catch::StringMaker<FlatpakApp::Branch> {
    static std::string convert(const FlatpakApp::Branch& branch) {
        return std::format(
            "{{name={}, installation={}, is_current={}}}", branch.name,
            branch.installation == FlatpakApp::Installation::User ? "User" : "System",
            branch.is_current
        );
    }
};

template <>
struct Catch::StringMaker<FlatpakApp> {
    static std::string convert(const FlatpakApp& app) {
        return std::format(
            "{{name={}, app_id={}, branches={}}}", app.name, app.app_id,
            Catch::Detail::stringify(app.branches)
        );
    }
};

// TODO: add tests for when an app is installed in both user and system
// these then each have their own current branch
// and add tests for malformed input
// probably just want empty vectors/skipped lines?

TEST_CASE("Parse valid flatpak list json", "[flatpakutils]") {
    static constexpr std::string_view json = R"([
        {
          "name" : "Flatseal",
          "application_id" : "com.github.tchx84.Flatseal",
          "branch" : "stable",
          "options" : "system,current",
          "installation" : "system"
        },
        {
          "name" : "Gear Lever",
          "application_id" : "it.mijorus.gearlever",
          "branch" : "stable",
          "options" : "system,current",
          "installation" : "system"
        },
        {
            "name" : "Odamex",
            "application_id" : "net.odamex.Odamex",
            "branch" : "stable",
            "options" : "user,current",
            "installation" : "user"
        },
        {
            "name" : "Odamex",
            "application_id" : "net.odamex.Odamex",
            "branch" : "stable",
            "options" : "system",
            "installation" : "system"
        },
        {
            "name" : "Odamex",
            "application_id" : "net.odamex.Odamex",
            "branch" : "protobreak",
            "options" : "system,current",
            "installation" : "system"
        },
        {
          "name" : "Thunderbird ESR",
          "application_id" : "org.mozilla.thunderbird_esr",
          "branch" : "stable",
          "options" : "system,current",
          "installation" : "system"
        }
    ])";
    const std::vector<FlatpakListEntry> expected{
        {.name = "Flatseal",
         .application_id = "com.github.tchx84.Flatseal",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
        {.name = "Gear Lever",
         .application_id = "it.mijorus.gearlever",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "user,current",
         .installation = "user"  },
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "system",
         .installation = "system"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "protobreak",
         .options = "system,current",
         .installation = "system"},
        {.name = "Thunderbird ESR",
         .application_id = "org.mozilla.thunderbird_esr",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
    };
    const auto actual = flatpakutil::detail::parse_apps_from_json(json);
    REQUIRE(actual == expected);
}

TEST_CASE("Parse valid flatpak list columns", "[flatpakutils]") {
    static constexpr std::string_view columns = R"(
        Flatseal	com.github.tchx84.Flatseal	stable	system,current	system
        Gear Lever	it.mijorus.gearlever	stable	system,current	system
        Odamex	net.odamex.Odamex	stable	user,current	user
        Odamex	net.odamex.Odamex	stable	system	system
        Odamex	net.odamex.Odamex	protobreak	system,current	system
        Thunderbird ESR	org.mozilla.thunderbird_esr	stable	system,current	system
    )";
    const std::vector<FlatpakListEntry> expected{
        {.name = "Flatseal",
         .application_id = "com.github.tchx84.Flatseal",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
        {.name = "Gear Lever",
         .application_id = "it.mijorus.gearlever",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "user,current",
         .installation = "user"  },
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "system",
         .installation = "system"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "protobreak",
         .options = "system,current",
         .installation = "system"},
        {.name = "Thunderbird ESR",
         .application_id = "org.mozilla.thunderbird_esr",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
    };
    const auto actual = flatpakutil::detail::parse_apps_from_columns(columns);
    REQUIRE(actual == expected);
}

TEST_CASE("Parsed list to app id map", "[flatpakutils]") {
    const std::vector<FlatpakListEntry> parsed_list{
        {.name = "Flatseal",
         .application_id = "com.github.tchx84.Flatseal",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
        {.name = "Gear Lever",
         .application_id = "it.mijorus.gearlever",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "user,current",
         .installation = "user"  },
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "system",
         .installation = "system"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "protobreak",
         .options = "system,current",
         .installation = "system"},
        {.name = "Thunderbird ESR",
         .application_id = "org.mozilla.thunderbird_esr",
         .branch = "stable",
         .options = "system,current",
         .installation = "system"},
    };
    const auto actual = flatpakutil::detail::apps_from_parsed_list(parsed_list);
    using enum flatpakutil::FlatpakApp::Installation;
    // clang-format off
    const std::unordered_map<std::string, FlatpakApp> expected{
        {
            "com.github.tchx84.Flatseal",
            {
                .name = "Flatseal",
                .app_id = "com.github.tchx84.Flatseal",
                .branches{{.name = "stable", .installation = System, .is_current = true}},
            },
        },
        {
            "it.mijorus.gearlever",
            {
                .name = "Gear Lever",
                .app_id = "it.mijorus.gearlever",
                .branches{{.name = "stable", .installation = System, .is_current = true}},
            },
        },
        {
            "net.odamex.Odamex",
            {
                .name = "Odamex",
                .app_id = "net.odamex.Odamex",
                .branches{
                    {.name = "stable", .installation = User, .is_current = true},
                    {.name = "stable", .installation = System, .is_current = false},
                    {.name = "protobreak", .installation = System, .is_current = true},
                },
            },
        },
        {
            "org.mozilla.thunderbird_esr",
            {
                .name = "Thunderbird ESR",
                .app_id = "org.mozilla.thunderbird_esr",
                .branches{{.name = "stable", .installation = System, .is_current = true}},
            },
        },
    };
    // clang-format on
    REQUIRE(actual.size() == expected.size());
    for (const auto& [id, app] : expected) {
        REQUIRE(actual.contains(id));
        REQUIRE(actual.at(id) == app);
    }
}
