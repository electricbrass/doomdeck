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

#include <experimental/scope>

#include <SDL3/SDL.h>
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_PNM
#define STBI_NO_PSD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module ui.images;

import std;
import imgui;
import imgui_impl_sdl3;
import imgui_impl_sdlgpu3;

import errors;
import icons;
import utils;

export namespace ui::image {

struct TextureData {
    SDL_GPUTexture* texture;
    int height;
    int width;
};

class TextureLoader {
private:
    // The largest texture we use is the playstation texture atlas
    // which is 1536x1536, and all textures we use are 4 byte RGBA
    static constexpr Uint32 transfer_buffer_size = static_cast<Uint32>(1536 * 1536 * 4);

    SDL_GPUDevice* m_device;
    SDL_GPUTransferBuffer* m_transfer_buffer;

public:
    explicit TextureLoader(SDL_GPUDevice* device) : m_device(device) {
        SDL_GPUTransferBufferCreateInfo transferbuffer_info{};
        transferbuffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferbuffer_info.size = transfer_buffer_size;
        m_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transferbuffer_info);
        if (!m_transfer_buffer) {
            throw errors::RuntimeError("Failed to create transfer buffer: {}", SDL_GetError());
        }
    }

    ~TextureLoader() { SDL_ReleaseGPUTransferBuffer(m_device, m_transfer_buffer); }

    TextureLoader(const TextureLoader&) = delete;
    auto operator=(const TextureLoader&) -> TextureLoader& = delete;
    // TODO: is there any reason to make this moveable?
    TextureLoader(TextureLoader&&) = delete;
    auto operator=(TextureLoader&&) -> TextureLoader& = delete;

    // TODO: better error handling here
    // maybe return std::expected instead of optional?
    auto load_texture(std::span<const std::uint8_t> data) -> std::optional<TextureData> {
        using std::experimental::scope_exit;
        int width, height; // NOLINT
        stbi_uc* image_data = stbi_load_from_memory(
            data.data(), static_cast<int>(data.size()), &width, &height, nullptr, STBI_rgb_alpha
        );

        if (!image_data) {
            return std::nullopt;
        }

        const scope_exit free_image([&] { stbi_image_free(image_data); });

        SDL_GPUTextureCreateInfo texture_info = {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = static_cast<Uint32>(width),
            .height = static_cast<Uint32>(height),
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
            .props = 0
        };

        const std::size_t image_size =
            static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4;

        // for now lets just not even try to support loading larger images
        if (image_size > transfer_buffer_size) {
            return std::nullopt;
        }

        SDL_GPUTexture* texture = SDL_CreateGPUTexture(m_device, &texture_info);
        if (!texture) {
            return std::nullopt;
        }

        scope_exit release_texture([&] { SDL_ReleaseGPUTexture(m_device, texture); });

        void* texture_ptr = SDL_MapGPUTransferBuffer(m_device, m_transfer_buffer, true);
        if (!texture_ptr) {
            return std::nullopt;
        }

        std::memcpy(texture_ptr, image_data, image_size);
        SDL_UnmapGPUTransferBuffer(m_device, m_transfer_buffer);

        SDL_GPUTextureTransferInfo transfer_info = {};
        transfer_info.offset = 0;
        transfer_info.transfer_buffer = m_transfer_buffer;

        SDL_GPUTextureRegion texture_region = {};
        texture_region.texture = texture;
        texture_region.x = 0;
        texture_region.y = 0;
        texture_region.w = static_cast<Uint32>(width);
        texture_region.h = static_cast<Uint32>(height);
        texture_region.d = 1;

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(m_device);
        if (!cmd) {
            return std::nullopt;
        }

        scope_exit cancel_command_buffer([&] { SDL_CancelGPUCommandBuffer(cmd); });

        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
        if (!copy_pass) {
            return std::nullopt;
        }

        SDL_UploadToGPUTexture(copy_pass, &transfer_info, &texture_region, false);
        SDL_EndGPUCopyPass(copy_pass);
        if (!SDL_SubmitGPUCommandBuffer(cmd)) {
            return std::nullopt;
        }

        release_texture.release();
        cancel_command_buffer.release();

        return std::make_optional(
            TextureData{.texture = texture, .height = height, .width = width}
        );
    }
};

class TextureManager {
public:
    using TextureID = std::uint64_t;

private:
    SDL_GPUDevice* m_device;
    TextureLoader m_textureloader;
    std::unordered_map<TextureID, TextureData> m_textures;
    std::array<std::optional<TextureID>, 4> m_controller_textures;
    TextureID m_next_id = 0;

    [[nodiscard]]
    static auto to_imgui_texture(SDL_GPUTexture* texture) -> ImTextureID {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return static_cast<ImTextureID>(reinterpret_cast<std::uintptr_t>(texture));
    }

public:
    explicit TextureManager(SDL_GPUDevice* device) : m_device(device), m_textureloader(device) {}

    ~TextureManager() {
        for (auto& [id, texture] : m_textures) {
            SDL_ReleaseGPUTexture(m_device, texture.texture);
        }
    }

    TextureManager(const TextureManager&) = delete;
    auto operator=(const TextureManager&) -> TextureManager& = delete;
    // TODO: is there any reason to make this moveable?
    TextureManager(TextureManager&&) = delete;
    auto operator=(TextureManager&&) -> TextureManager& = delete;

    auto load_texture(std::span<const std::uint8_t> data) -> std::optional<TextureID> {
        const auto texture = m_textureloader.load_texture(data);
        if (!texture) {
            return std::nullopt;
        }

        const auto id = m_next_id++;
        m_textures.emplace(id, *texture);
        return id;
    }

    void load_static_textures() {
        using enum icons::ControllerType;
        m_controller_textures[static_cast<std::size_t>(Steam)] =
            load_texture(icons::steam_atlas.png);
        m_controller_textures[static_cast<std::size_t>(Nintendo)] =
            load_texture(icons::nintendo_atlas.png);
        m_controller_textures[static_cast<std::size_t>(Xbox)] = load_texture(icons::xbox_atlas.png);
        m_controller_textures[static_cast<std::size_t>(Playstation)] =
            load_texture(icons::playstation_atlas.png);
    }

    [[nodiscard]]
    auto get_texture(TextureID id) const -> std::optional<ImTextureID> {
        const auto it = m_textures.find(id);
        if (it == m_textures.end()) {
            return std::nullopt;
        }

        return to_imgui_texture(it->second.texture);
    }

    [[nodiscard]]
    auto get_controller_texture(icons::ControllerType type) const -> std::optional<ImTextureID> {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        const auto id = m_controller_textures[static_cast<std::size_t>(type)];
        if (!id) {
            return std::nullopt;
        }

        return get_texture(*id);
    }
};

} // namespace ui::image
