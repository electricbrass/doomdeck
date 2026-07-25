#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_tostring.hpp>

import std;
import flatpakutils;

using flatpakutil::FlatpakApp;
using flatpakutil::detail::FlatpakJSON;

template <>
struct Catch::StringMaker<FlatpakJSON> {
    static std::string convert(const FlatpakJSON& ref) {
        return std::format(
            "{{name={}, application_id={}, branch={}, options={}}}", ref.name, ref.application_id,
            ref.branch, ref.options
        );
    }
};

template <>
struct Catch::StringMaker<FlatpakApp> {
    static std::string convert(const FlatpakApp& app) {
        return std::format(
            "{{name={}, app_id={}, branches={}, current_branch={}}}", app.name, app.app_id,
            app.branches, app.current_branch
        );
    }
};

TEST_CASE("Parse valid flatpak list json", "[flatpakutils]") {
    static constexpr std::string_view json = R"([
        {
          "name" : "Flatseal",
          "application_id" : "com.github.tchx84.Flatseal",
          "branch" : "stable",
          "options" : "system,current"
        },
        {
          "name" : "Gear Lever",
          "application_id" : "it.mijorus.gearlever",
          "branch" : "stable",
          "options" : "system,current"
        },
        {
            "name" : "Odamex",
            "application_id" : "net.odamex.Odamex",
            "branch" : "stable",
            "options" : "system"
        },
        {
            "name" : "Odamex",
            "application_id" : "net.odamex.Odamex",
            "branch" : "protobreak",
            "options" : "system,current"
        },
        {
          "name" : "Thunderbird ESR",
          "application_id" : "org.mozilla.thunderbird_esr",
          "branch" : "stable",
          "options" : "system,current"
        }
    ])";
    const std::vector<FlatpakJSON> expected{
        {.name = "Flatseal",
         .application_id = "com.github.tchx84.Flatseal",
         .branch = "stable",
         .options = "system,current"},
        {.name = "Gear Lever",
         .application_id = "it.mijorus.gearlever",
         .branch = "stable",
         .options = "system,current"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "system"        },
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "protobreak",
         .options = "system,current"},
        {.name = "Thunderbird ESR",
         .application_id = "org.mozilla.thunderbird_esr",
         .branch = "stable",
         .options = "system,current"},
    };
    const auto actual = flatpakutil::detail::parse_apps_from_json(json);
    REQUIRE(actual == expected);
    const auto actual2 = flatpakutil::detail::apps_json_to_whatever(actual);
    const std::unordered_map<std::string, FlatpakApp> expected2{
        {"com.github.tchx84.Flatseal",
         {.name = "Flatseal",
          .app_id = "com.github.tchx84.Flatseal",
          .branches{"stable"},
          .current_branch = "stable"}    },
        {"it.mijorus.gearlever",
         {.name = "Gear Lever",
          .app_id = "it.mijorus.gearlever",
          .branches{"stable"},
          .current_branch = "stable"}    },
        {"net.odamex.Odamex",
         {.name = "Odamex",
          .app_id = "net.odamex.Odamex",
          .branches{"stable", "protobreak"},
          .current_branch = "protobreak"}},
        {"org.mozilla.thunderbird_esr",
         {.name = "Thunderbird ESR",
          .app_id = "org.mozilla.thunderbird_esr",
          .branches{"stable"},
          .current_branch = "stable"}    },
    };
    REQUIRE(actual2.size() == expected2.size());
    for (const auto& [id, app] : expected2) {
        REQUIRE(actual2.contains(id));
        REQUIRE(actual2.at(id) == app);
    }
}

TEST_CASE("Parse valid flatpak list columns", "[flatpakutils]") {
    static constexpr std::string_view columns = R"(
        Flatseal	com.github.tchx84.Flatseal	stable	system,current
        Gear Lever	it.mijorus.gearlever	stable	system,current
        Odamex	net.odamex.Odamex	stable	system
        Odamex	net.odamex.Odamex	protobreak	system,current
        Thunderbird ESR	org.mozilla.thunderbird_esr	stable	system,current
    )";
    const std::vector<FlatpakJSON> expected{
        {.name = "Flatseal",
         .application_id = "com.github.tchx84.Flatseal",
         .branch = "stable",
         .options = "system,current"},
        {.name = "Gear Lever",
         .application_id = "it.mijorus.gearlever",
         .branch = "stable",
         .options = "system,current"},
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "stable",
         .options = "system"        },
        {.name = "Odamex",
         .application_id = "net.odamex.Odamex",
         .branch = "protobreak",
         .options = "system,current"},
        {.name = "Thunderbird ESR",
         .application_id = "org.mozilla.thunderbird_esr",
         .branch = "stable",
         .options = "system,current"},
    };
    const auto actual = flatpakutil::detail::parse_apps_from_columns(columns);
    REQUIRE(actual == expected);
}
