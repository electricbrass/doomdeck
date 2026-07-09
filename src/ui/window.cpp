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

export module ui.window;

import std;
import imgui;

import appinfo;
import appstate;
import ui.settings;
import ui.types;

namespace ui {

void draw_header() {
    ImGui::TextUnformatted("DoomDeck");

    ImGui::SameLine();

    ImGui::TextDisabled(appinfo::version_string_cstr);
}

struct TabInfo {
    const char* title;
    Tab tab;
};

constexpr std::array<TabInfo, 5> tabs = {
    {
     {.title = "Launcher", .tab = Tab::Launcher},
     {.title = "PWADs", .tab = Tab::Pwads},
     {.title = "IWADs", .tab = Tab::Iwads},
     {.title = "Ports", .tab = Tab::Ports},
     {.title = "Settings", .tab = Tab::Settings},
     }
};

void draw_tabs(Tab& current_tab) {
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const std::size_t count = std::size(tabs);

    float available = ImGui::GetContentRegionAvail().x;
    float width = (available - (spacing * (count - 1))) / count;

    ImGui::BeginGroup();
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);

    for (const auto [title, tab] : tabs) {
        const bool active = current_tab == tab;

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        // TODO: be smarter about this
        const float button_height = 48;

        if (ImGui::Button(title, {width, button_height})) {
            current_tab = tab;
        }

        if (active) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
    }

    ImGui::PopItemFlag();
    ImGui::EndGroup();
}

void draw_footer(bool& should_quit) {
    if (ImGui::Button("Quit")) {
        should_quit = true;
    }
}

export bool draw_window(ApplicationState& state) {
    bool should_quit = false;

    draw_header();

    draw_tabs(state.current_tab);

    ImGui::Separator();

    const float footer_height =
        ImGui::GetFrameHeightWithSpacing() + (ImGui::GetStyle().ItemSpacing.y * 2);

    ImGui::BeginChild("Content", ImVec2(0, -footer_height));
    switch (state.current_tab) {
        case Tab::Launcher:
            break;
        case Tab::Pwads:
            break;
        case Tab::Iwads:
            break;
        case Tab::Ports:
            break;
        case Tab::Settings:
            settings_tab(state);
            break;
    }
    ImGui::EndChild();

    ImGui::Separator();

    draw_footer(should_quit);

    return should_quit;
}

} // namespace ui
