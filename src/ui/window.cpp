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

void draw_header() {
    ImGui::TextUnformatted("DoomDeck");

    ImGui::SameLine();

    ImGui::TextDisabled(appinfo::version_string_cstr);

    // TODO: draw controller icons for L + R indicating the tabs
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
    // ImGui::SameLine();

    const icons::Atlas* idk;
    switch (controller) {
        case icons::ControllerType::Steam:
            idk = &icons::steam_atlas;
            break;
        case icons::ControllerType::Nintendo:
            idk = &icons::nintendo_atlas;
            break;
        case icons::ControllerType::Xbox:
            idk = &icons::xbox_atlas;
            break;
        case icons::ControllerType::Playstation:
            idk = &icons::playstation_atlas;
            break;
    }

    // if (auto tex = image::load_image(idk->png, gpu_device)) {
    if (auto tex = texture_manager.get_controller_texture(controller)) {
        // if (false) {
        constexpr int icon_size = 128;
        auto icon = (*idk)
            [ImGui::GetIO().ConfigNavSwapGamepadButtons ? icons::ControllerButton::FaceRight
                                                        : icons::ControllerButton::FaceBottom];
        ImVec2 uv0;
        ImVec2 uv1;
        uv0.x = float(icon.x) / idk->width;
        uv0.y = float(idk->height - icon.y - icon_size) / idk->height;
        uv1.x = float(icon.x + icon_size) / idk->width;
        uv1.y = float(idk->height - icon.y) / idk->height;
        auto height = ImGui::GetFrameHeight();

        ImGui::Image(*tex, {height, height}, uv0, uv1);
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("Select");
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    // if (auto tex = image::load_image(idk->png, gpu_device)) {
    if (auto tex = texture_manager.get_controller_texture(controller)) {
        // if (false) {
        constexpr int icon_size = 128;
        auto icon = (*idk)
            [ImGui::GetIO().ConfigNavSwapGamepadButtons ? icons::ControllerButton::FaceBottom
                                                        : icons::ControllerButton::FaceRight];
        ImVec2 uv0;
        ImVec2 uv1;
        uv0.x = float(icon.x) / idk->width;
        uv0.y = float(idk->height - icon.y - icon_size) / idk->height;
        uv1.x = float(icon.x + icon_size) / idk->width;
        uv1.y = float(idk->height - icon.y) / idk->height;
        auto height = ImGui::GetFrameHeight();

        ImGui::Image(*tex, {height, height}, uv0, uv1);
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("Cancel");
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

    draw_footer(should_quit, state.config.settings.controller_type, state.texture_manager);

    return should_quit;
}

} // namespace ui
