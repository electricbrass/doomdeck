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

export module ui.settings;

import std;
import imgui;

import appinfo;
import appstate;
import config;

namespace ui {

const std::string full_version = std::format(
    "v{}+{}{}", appinfo::version_string, appinfo::git::commit,
    appinfo::git::is_dirty ? ".dirty" : ""
);

void centered_text(const char* text) {
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_size.x) * 0.5f);
    ImGui::TextUnformatted(text);
}

void centered_text_disabled(const char* text) {
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_size.x) * 0.5f);
    ImGui::TextDisabled("%s", text);
}

void centered_url(const char* text, const char* url = nullptr) {
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_size.x) * 0.5f);
    ImGui::TextLinkOpenURL(text, url);
}

void about_section() {
    ImGui::TextUnformatted("About:");
    centered_text(appinfo::name_cstr);
    centered_text_disabled(full_version.c_str());
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.8f);
    // TODO: have the url come from cmake too
    centered_url("Source repository", "https://github.com/electricbrass/doomdeck");
    // TODO: and the copyright string too
    centered_text_disabled("© 2026 Mia McMahill <electricbrass@proton.me>");
    ImGui::PopFont();
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.64f);
    centered_text_disabled("Licensed under the GNU General Public License, version 3");
    ImGui::PopFont();
}

export void settings_tab(ApplicationState& state) {
    const ImVec2 size = ImGui::GetContentRegionAvail();

    const float spacing = ImGui::GetStyle().ItemSpacing.x;

    const float left_width = (size.x - spacing) * 0.5f;
    const float right_width = size.x - spacing - left_width;

    ImGui::BeginChild("LeftPane", {left_width, size.y}, ImGuiChildFlags_Borders);
    ImGui::Checkbox("Demo Window", &state.show_demo_window);
    ImGui::SeparatorText("Interface");
    if (ImGui::Checkbox("Fullscreen", &state.config.settings.fullscreen)) {
        if (!SDL_SetWindowFullscreen(state.window, state.config.settings.fullscreen)) {
            // TODO: log error
            state.config.settings.fullscreen = !state.config.settings.fullscreen;
        }
    }
    if (ImGui::Checkbox("Swap Face Buttons", &state.config.settings.swap_face_buttons)) {
        ImGui::GetIO().ConfigNavSwapGamepadButtons = state.config.settings.swap_face_buttons;
    }
    // TODO: clean up these combos
    std::array<const char*, 4> button_labels = {"Steam", "Nintendo", "Xbox", "Playstation"};
    if (ImGui::BeginCombo(
            "Button Labels",
            button_labels[static_cast<size_t>(state.config.settings.controller_type)],
            ImGuiComboFlags_WidthFitPreview
        )) {
        for (size_t i = 0; i < 4; i++) {
            const bool is_selected =
                (static_cast<size_t>(state.config.settings.controller_type) == i);
            if (ImGui::Selectable(button_labels[i])) {
                state.config.settings.controller_type = static_cast<config::ControllerType>(i);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SeparatorText("Other stuff idk");
    std::array<const char*, 2> thumbnail_labels = {"Titlepic", "Endoom"};
    if (ImGui::BeginCombo(
            "Thumbnail Type",
            thumbnail_labels[static_cast<size_t>(state.config.settings.thumbnail_type)],
            ImGuiComboFlags_WidthFitPreview
        )) {
        for (size_t i = 0; i < 2; i++) {
            const bool is_selected =
                (static_cast<size_t>(state.config.settings.thumbnail_type) == i);
            if (ImGui::Selectable(thumbnail_labels[i])) {
                state.config.settings.thumbnail_type = static_cast<config::ThumbnailType>(i);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("RightPane", {right_width, size.y}, ImGuiChildFlags_None);

    const float top_height = (size.y + spacing) * 0.5f;

    ImGui::BeginChild("RightTop", {0, top_height}, ImGuiChildFlags_Borders);

    // top right content

    ImGui::EndChild();

    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
    ImGui::BeginChild("RightBottom", {0, 0}, ImGuiChildFlags_Borders);

    about_section();

    ImGui::EndChild();
    ImGui::PopItemFlag();

    ImGui::EndChild();
}

} // namespace ui
