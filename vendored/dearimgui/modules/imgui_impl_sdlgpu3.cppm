module;

#include <imgui_impl_sdlgpu3.h>

export module imgui_impl_sdlgpu3;

export import imgui;

export {
    // ----- Types -----

    using ::ImDrawData;
    using ::ImGui_ImplSDLGPU3_InitInfo;
    using ::ImTextureData;
    using ::SDL_GPUCommandBuffer;
    using ::SDL_GPUGraphicsPipeline;
    using ::SDL_GPURenderPass;

    // ----- Functions -----

    using ::ImGui_ImplSDLGPU3_CreateDeviceObjects;
    using ::ImGui_ImplSDLGPU3_DestroyDeviceObjects;
    using ::ImGui_ImplSDLGPU3_Init;
    using ::ImGui_ImplSDLGPU3_NewFrame;
    using ::ImGui_ImplSDLGPU3_PrepareDrawData;
    using ::ImGui_ImplSDLGPU3_RenderDrawData;
    using ::ImGui_ImplSDLGPU3_Shutdown;
    using ::ImGui_ImplSDLGPU3_UpdateTexture;
}
