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

export module appstate;

import config;
import ui.images;
import ui.types;

export struct ApplicationState {
    config::Config config;
    bool show_demo_window = false;
    SDL_Window* window = nullptr;
    ui::Tab current_tab = ui::Tab::Launcher;
    SDL_GPUDevice* gpu_device = nullptr;
    ui::image::TextureManager texture_manager;
};
