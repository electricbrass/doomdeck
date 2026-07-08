add_library(dearimgui OBJECT)

target_sources(dearimgui
  PRIVATE
    src/imgui.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui_impl_sdl3.cpp
    src/imgui_impl_sdlgpu3.cpp
  PUBLIC FILE_SET CXX_MODULES FILES
    modules/imgui.cppm
    modules/imgui_internal.cppm
    modules/imgui_impl_sdl3.cppm
    modules/imgui_impl_sdlgpu3.cppm
)

target_include_directories(dearimgui PUBLIC
  include
  SYSTEM
)

target_compile_definitions(dearimgui
  PUBLIC
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
)

target_compile_features(dearimgui
  PUBLIC
    cxx_std_23
)
