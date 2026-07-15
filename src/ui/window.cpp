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

#include <SDL3/SDL.h>

export module ui.window;

import std;
import imgui;

import appinfo;
import appstate;
import icons;
import ui.images;
import ui.settings;
import ui.types;

namespace ui {

namespace {

void draw_controller_prompt(
    icons::ControllerButton button, std::string_view text, icons::ControllerType controller,
    image::TextureManager& texture_manager
) {
    const icons::Atlas* atlas = nullptr;
    switch (controller) {
        case icons::ControllerType::Steam:
            atlas = &icons::steam_atlas;
            break;
        case icons::ControllerType::Nintendo:
            atlas = &icons::nintendo_atlas;
            break;
        case icons::ControllerType::Xbox:
            atlas = &icons::xbox_atlas;
            break;
        case icons::ControllerType::Playstation:
            atlas = &icons::playstation_atlas;
            break;
    }
    ImGui::BeginGroup();
    if (auto tex = texture_manager.get_controller_texture(controller)) {
        const auto icon = (*atlas)[button];
        const auto height = ImGui::GetFrameHeight();

        ImGui::Image(*tex, {height, height}, {icon.x0, icon.y0}, {icon.x1, icon.y1});
    }

    ImGui::SameLine();
    ImGui::TextUnformatted(text.begin(), text.end());
    ImGui::EndGroup();
}

} // namespace

void draw_header(icons::ControllerType controller, image::TextureManager& texture_manager) {
    ImGui::TextUnformatted(appinfo::name_cstr);

    ImGui::SameLine();

    ImGui::TextDisabled(appinfo::version_string_cstr);

    const auto height = ImGui::GetFrameHeight();

    const float spacing =
        ImGui::GetStyle().ItemSpacing.x * 3 + ImGui::GetStyle().FramePadding.x * 2;
    const float text_width = ImGui::CalcTextSize("Prev Tab").x + ImGui::CalcTextSize("Next Tab").x;
    const float group_width = (ImGui::GetFrameHeight() * 2) + spacing + text_width;
    ImGui::SameLine();

    const float right_edge = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right_edge - group_width);

    // TODO: figure out how to avoid needing to do this
    const float y = ImGui::GetCursorPosY() + ((ImGui::GetItemRectSize().y - height) * 0.5f);
    ImGui::SetCursorPosY(y);

    draw_controller_prompt(
        icons::ControllerButton::LeftShoulder, "Prev Tab", controller, texture_manager
    );

    ImGui::SameLine();
    ImGui::SetCursorPosY(y);

    draw_controller_prompt(
        icons::ControllerButton::RightShoulder, "Next Tab", controller, texture_manager
    );
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

void draw_footer(
    bool& should_quit, icons::ControllerType controller, image::TextureManager& texture_manager
) {
    if (ImGui::Button("Quit")) {
        should_quit = true;
    }

    // TODO: make sure this calculation is correct
    // and only draw if currently using a controller
    const float spacing =
        ImGui::GetStyle().ItemSpacing.x * 3 + ImGui::GetStyle().FramePadding.x * 2;
    const float text_width = ImGui::CalcTextSize("Select").x + ImGui::CalcTextSize("Cancel").x;
    const float group_width = (ImGui::GetFrameHeight() * 2) + spacing + text_width;
    ImGui::SameLine();

    const float right_edge = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right_edge - group_width);

    draw_controller_prompt(
        ImGui::GetIO().ConfigNavSwapGamepadButtons ? icons::ControllerButton::FaceRight
                                                   : icons::ControllerButton::FaceBottom,
        "Select", controller, texture_manager
    );

    ImGui::SameLine();

    draw_controller_prompt(
        ImGui::GetIO().ConfigNavSwapGamepadButtons ? icons::ControllerButton::FaceBottom
                                                   : icons::ControllerButton::FaceRight,
        "Cancel", controller, texture_manager
    );
}

export auto draw_window(ApplicationState& state) -> bool {
    bool should_quit = false;

    draw_header(state.config.settings.controller_type, state.texture_manager);

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

    draw_footer(should_quit, state.config.settings.controller_type, state.texture_manager);

    return should_quit;
}

} // namespace ui
