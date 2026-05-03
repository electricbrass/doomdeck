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

#include <RmlUi/Core.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi_Backend.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

import std;
import foo;

namespace {
struct ApplicationData {
    bool show_text = true;
    Rml::String animal = "dog";
} my_data;
} // namespace

auto main() -> int {
    const int window_width = 1024;
    const int window_height = 768;

    Backend::Initialize("my cool window", window_width, window_height, true);

    // Install the custom interfaces.
    Rml::SetRenderInterface(Backend::GetRenderInterface());
    Rml::SetSystemInterface(Backend::GetSystemInterface());

    // Now we can initialize RmlUi.
    Rml::Initialise();

    // Create a context to display documents within.
    Rml::Context* context =
        Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));

    // Tell RmlUi to load the given fonts.
    Rml::LoadFontFace("Lato-Regular.ttf");
    // Fonts can be registered as fallback fonts, as in this case to display
    // emojis.
    Rml::LoadFontFace("NotoColorEmoji.ttf", true);

    // Set up data bindings to synchronize application data.
    if (Rml::DataModelConstructor constructor =
            context->CreateDataModel("animals")) {
        constructor.Bind("show_text", &my_data.show_text);
        constructor.Bind("animal", &my_data.animal);
    }

    // Now we are ready to load our document.
    Rml::ElementDocument* document = context->LoadDocument("src/ui/main.rml");
    document->Show();

    // Replace and style some text in the loaded document.
    Rml::Element* element = document->GetElementById("world");
    element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
    element->SetProperty("font-size", "1.5em");

    bool exit_application = false;
    while (!exit_application) {
        exit_application = !Backend::ProcessEvents(context);
        // We assume here that we have some way of updating and retrieving
        // inputs internally.
        // if (my_input->KeyPressed(KEY_ESC))
        //     exit_application = true;

        // // Submit input events such as MouseMove and key events (not shown)
        // to the context. if (my_input->MouseMoved())
        //     context->ProcessMouseMove(mouse_pos.x, mouse_pos.y, 0);

        // Update the context to reflect any changes resulting from input
        // events, animations, modified and added elements, or changed data in
        // data bindings.
        context->Update();

        // Prepare the application for rendering, such as by clearing the
        // window. This calls into the RmlUi backend interface, replace with
        // your own procedures as appropriate.
        Backend::BeginFrame();

        // Render the user interface. All geometry and other rendering commands
        // are now submitted through the render interface.
        context->Render();

        // Present the rendered content, such as by swapping the swapchain. This
        // calls into the RmlUi backend interface, replace with your own
        // procedures as appropriate.
        Backend::PresentFrame();
    }

    Rml::Shutdown();
}
