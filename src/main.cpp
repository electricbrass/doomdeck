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

#include <SDL3/SDL.h>

import std;
import imgui_impl_sdl3;
import imgui_impl_sdlgpu3;

import appinfo;
import appstate;
import config;
import directories;
import errors;
import ui.images;
import ui.types;
import ui.window;

namespace {

auto handle_event(SDL_Event event, ApplicationState& state) -> bool {
    const auto toggle_fullscreen = [&]() {
        if (!SDL_SetWindowFullscreen(state.window, !state.config.settings.fullscreen)) {
            // TODO: log error
        } else {
            state.config.settings.fullscreen = !state.config.settings.fullscreen;
        }
    };
    // NOLINTNEXTLINE(bugprone-switch-missing-default-case)
    switch (event.type) {
        case SDL_EVENT_QUIT:
            return true;
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (event.window.windowID == SDL_GetWindowID(state.window)) {
                return true;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            // if (io.WantCaptureKeyboard) {
            //     break;
            // }
            if (!event.key.repeat) {
                // NOLINTNEXTLINE(bugprone-switch-missing-default-case)
                switch (event.key.key) {
                    case SDLK_ESCAPE:
                        return true;
                        break;
                    case SDLK_F11:
                        toggle_fullscreen();
                        break;
                    case SDLK_RETURN:
                        if (event.key.mod & SDL_KMOD_ALT) {
                            toggle_fullscreen();
                        }
                        break;
                }
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            switch (event.gbutton.button) {
                case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
                    state.current_tab = ui::prev_tab(state.current_tab);
                    break;
                case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
                    state.current_tab = ui::next_tab(state.current_tab);
                    break;
            }
            break;
    }
    return false;
}

} // namespace

auto main() -> int {
    SDL_SetHintWithPriority(SDL_HINT_APP_ID, appinfo::app_id_cstr, SDL_HINT_OVERRIDE);
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::println("Error: SDL_Init(): {}", SDL_GetError());
        return 1;
    }

    const float window_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    const float main_scale = window_scale * 2;
    SDL_WindowFlags window_flags =
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(
        appinfo::name_cstr, static_cast<int>(1280 * window_scale),
        static_cast<int>(800 * window_scale), window_flags
    );
    if (window == nullptr) {
        std::println("Error: SDL_CreateWindow(): {}", SDL_GetError());
        return 1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL |
            SDL_GPU_SHADERFORMAT_METALLIB,
        true, nullptr
    );
    if (gpu_device == nullptr) {
        std::println("Error: SDL_CreateGPUDevice(): {}", SDL_GetError());
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
        std::println("Error: SDL_ClaimWindowForGPUDevice(): {}", SDL_GetError());
        return 1;
    }
    SDL_SetGPUSwapchainParameters(
        gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC
    );
    ImGui::CheckVersion();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.Fonts->AddFontDefaultVector();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    // set up style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    static constexpr float corner_rounding = 6.0f;
    style.WindowRounding = corner_rounding;
    style.ChildRounding = corner_rounding;
    style.FrameRounding = corner_rounding;
    style.PopupRounding = corner_rounding;
    style.GrabRounding = corner_rounding;
    ImGui_ImplSDL3_InitForSDLGPU(window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {
        .Device = gpu_device,
        .ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window),
    };
    ImGui_ImplSDLGPU3_Init(&init_info);

    // TODO: split this up some into multiple functions
    {
        const dirs::Directories dirs{};
        ApplicationState state{
            .config = config::Config::load(""),
            .texture_manager = ui::image::TextureManager(gpu_device),
        };
        state.window = window;
        state.gpu_device = gpu_device;
        io.ConfigNavSwapGamepadButtons = state.config.settings.swap_face_buttons;
        state.texture_manager.load_static_textures();

        // main loop
        bool done = false;
        while (!done) {
            try {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL3_ProcessEvent(&event);
                    done |= handle_event(event, state);
                }

                // skip when minimized
                if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
                    SDL_Delay(10);
                    continue;
                }

                // Start the Dear ImGui frame
                ImGui_ImplSDLGPU3_NewFrame();
                ImGui_ImplSDL3_NewFrame();
                ImGui::NewFrame();

                // main gui
                {
                    const ImGuiViewport* viewport = ImGui::GetMainViewport();

                    ImGui::SetNextWindowPos(viewport->Pos);
                    ImGui::SetNextWindowSize(viewport->Size);

                    const ImGuiWindowFlags flags =
                        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                        ImGuiWindowFlags_NoBackground;
                    ImGui::Begin("Main Window", nullptr, flags);

                    done |= ui::draw_window(state);

                    ImGui::End();
                }

                if (state.show_demo_window) {
                    ImGui::ShowDemoWindow(&state.show_demo_window);
                }

                ImGui::Render();
                ImDrawData* draw_data = ImGui::GetDrawData();
                const bool is_minimized =
                    (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

                SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);

                SDL_GPUTexture* swapchain_texture = nullptr;
                SDL_WaitAndAcquireGPUSwapchainTexture(
                    command_buffer, window, &swapchain_texture, nullptr, nullptr
                );

                if (swapchain_texture != nullptr && !is_minimized) {
                    // this is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the
                    // vertex/index buffer!
                    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

                    ImVec4 clear_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

                    // set up and start a render pass
                    SDL_GPUColorTargetInfo target_info = {};
                    target_info.texture = swapchain_texture;
                    target_info.mip_level = 0;
                    target_info.layer_or_depth_plane = 0;
                    target_info.clear_color = SDL_FColor{
                        .r = clear_color.x,
                        .g = clear_color.y,
                        .b = clear_color.z,
                        .a = clear_color.w
                    };
                    target_info.load_op = SDL_GPU_LOADOP_CLEAR;
                    target_info.store_op = SDL_GPU_STOREOP_STORE;
                    target_info.cycle = false;
                    SDL_GPURenderPass* render_pass =
                        SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

                    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);

                    SDL_EndGPURenderPass(render_pass);
                }

                SDL_SubmitGPUCommandBuffer(command_buffer);
            } catch (const errors::RuntimeError& e) {
                std::println("Runtime error: {}", e.what());
                std::println("Stacktrace:\n{}", e.stacktrace());
                done = true;
            } catch (const errors::LogicError& e) {
                std::println("Logic error: {}", e.what());
                std::println("Stacktrace:\n{}", e.stacktrace());
                done = true;
            } catch (const std::runtime_error& e) {
                std::println("Runtime error: {}", e.what());
                done = true;
            } catch (const std::logic_error& e) {
                std::println("Logic error: {}", e.what());
                done = true;
            } catch (const std::exception& e) {
                std::println("Exception: {}", e.what());
                done = true;
            }
        }
    }

    // cleanup
    SDL_WaitForGPUIdle(gpu_device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    SDL_DestroyGPUDevice(gpu_device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
