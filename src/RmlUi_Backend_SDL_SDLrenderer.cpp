#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Log.h>

import std;

namespace {

class BetterLogging : public SystemInterface_SDL {
public:
    explicit BetterLogging(SDL_Window* window) : SystemInterface_SDL(window) {}

    auto LogMessage(Rml::Log::Type /*type*/, const Rml::String& message) -> bool override {
        // TODO: maybe create log file and/or have current session log visible in the ui
        std::println("{}", message);
        return true;
    }
};

/**
 * Global data used by this backend.
 *
 * Lifetime governed by the calls to Backend::Initialize() and
 * Backend::Shutdown().
 */
struct BackendData {
    BackendData(SDL_Window* window, SDL_Renderer* renderer)
        : system_interface(window), render_interface(renderer) {}

    BetterLogging system_interface;
    RenderInterface_SDL render_interface;
    TextInputMethodEditor_SDL text_input_method_editor;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = true;
};
Rml::UniquePtr<BackendData> data; // NOLINT

} // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Backend::Initialize(const char* window_name, int width, int height, bool allow_resize)
    -> bool {
    RMLUI_ASSERT(!data);

    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return false;
    }

    // Submit click events when focusing the window.
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    // Touch events are handled natively, no need to generate synthetic mouse
    // events for touch devices.
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

#if defined RMLUI_BACKEND_SIMULATE_TOUCH
    // Simulate touch events from mouse events for testing touch behavior on a
    // desktop machine.
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
#endif

    const float window_size_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, window_name);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,
                          int(static_cast<float>(width) * window_size_scale));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,
                          int(static_cast<float>(height) * window_size_scale));
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, allow_resize);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
    SDL_Window* window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!window) {
        Rml::Log::Message(Rml::Log::LT_ERROR, "SDL error on create window: %s\n", SDL_GetError());
        return false;
    }

    // Force a specific SDL renderer
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d12");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        return false;
    }

    data = Rml::MakeUnique<BackendData>(window, renderer);
    data->window = window;
    data->renderer = renderer;

    const char* renderer_name = nullptr;
    Rml::String available_renderers;

    SDL_SetRenderVSync(renderer, 1);
    renderer_name = SDL_GetRendererName(renderer);
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
        if (i > 0) {
            available_renderers += ", ";
        }

        available_renderers += SDL_GetRenderDriver(i);
    }

    if (!available_renderers.empty()) {
        data->system_interface.LogMessage(
            Rml::Log::LT_INFO,
            Rml::CreateString("Available SDL renderers: %s", available_renderers.c_str()));
    }

    if (renderer_name) {
        data->system_interface.LogMessage(
            Rml::Log::LT_INFO, Rml::CreateString("Using SDL renderer: %s", renderer_name));
    }

    Rml::SetTextInputHandler(&data->text_input_method_editor);

    return true;
}

void Backend::Shutdown() {
    RMLUI_ASSERT(data);

    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);

    data.reset();

    SDL_Quit();
}

auto Backend::GetSystemInterface() -> Rml::SystemInterface* {
    RMLUI_ASSERT(data);
    return &data->system_interface;
}

auto Backend::GetRenderInterface() -> Rml::RenderInterface* {
    RMLUI_ASSERT(data);
    return &data->render_interface;
}

auto Backend::ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback,
                            bool power_save) -> bool {
    RMLUI_ASSERT(data && context);

    const auto get_key = [](const SDL_Event& event) { return event.key.key; };
    const auto get_display_scale = []() { return SDL_GetWindowDisplayScale(data->window); };
    constexpr auto event_quit = SDL_EVENT_QUIT;
    constexpr auto event_key_down = SDL_EVENT_KEY_DOWN;
    constexpr auto event_text_editing = SDL_EVENT_TEXT_EDITING;
    bool has_event = false;

    bool result = data->running;
    data->running = true;

    SDL_Event ev;
    if (power_save) {
        // NOLINTBEGIN
        // for the magic numbers, they're obvious
        has_event = SDL_WaitEventTimeout(
            &ev, static_cast<int>(Rml::Math::Min(context->GetNextUpdateDelay(), 10.0) * 1000));
        // NOLINTEND
    } else {
        has_event = SDL_PollEvent(&ev);
    }

    while (has_event) {
        bool propagate_event = true;
        switch (ev.type) {
            case event_quit: {
                propagate_event = false;
                result = false;
            } break;
            case event_key_down: {
                propagate_event = false;
                const Rml::Input::KeyIdentifier key =
                    RmlSDL::ConvertKey(static_cast<int>(get_key(ev)));
                const int key_modifier = RmlSDL::GetKeyModifierState();
                const float native_dp_ratio = get_display_scale();

                // See if we have any global shortcuts that take priority over the
                // context.
                if (key_down_callback &&
                    !key_down_callback(context, key, key_modifier, native_dp_ratio, true)) {
                    break;
                }

                // Otherwise, hand the event over to the context by calling the
                // input handler as normal.
                if (!RmlSDL::InputEventHandler(context, data->window, ev)) {
                    break;
                }

                // The key was not consumed by the context either, try keyboard
                // shortcuts of lower priority.
                if (key_down_callback &&
                    !key_down_callback(context, key, key_modifier, native_dp_ratio, false)) {
                    break;
                }
            } break;
            case event_text_editing: {
                propagate_event = false;
                data->text_input_method_editor.HandleEdit(ev.edit);
            } break;
            default:
                break;
        }

        if (propagate_event) {
            RmlSDL::InputEventHandler(context, data->window, ev);
        }

        has_event = SDL_PollEvent(&ev);
    }

    return result;
}

void Backend::RequestExit() {
    RMLUI_ASSERT(data);
    data->running = false;
}

void Backend::BeginFrame() {
    RMLUI_ASSERT(data);
    data->render_interface.BeginFrame();
}

void Backend::PresentFrame() {
    RMLUI_ASSERT(data);
    data->render_interface.EndFrame();
    SDL_RenderPresent(data->renderer);
}
