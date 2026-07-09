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
    src/misc/freetype/imgui_freetype.cpp
  PUBLIC FILE_SET CXX_MODULES FILES
    modules/imgui.cppm
    modules/imgui_freetype.cppm
    modules/imgui_internal.cppm
    modules/imgui_impl_sdl3.cppm
    modules/imgui_impl_sdlgpu3.cppm
)

target_include_directories(dearimgui PUBLIC
  include
  src/misc/freetype
  SYSTEM
)

target_compile_definitions(dearimgui
  PUBLIC
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_ENABLE_FREETYPE
    IMGUI_ENABLE_STB_TRUETYPE
)

target_compile_features(dearimgui
  PUBLIC
    cxx_std_23
)

FetchContent_Declare(
  Freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
  GIT_TAG VER-2-14-3
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Freetype)
target_link_libraries(dearimgui PRIVATE Freetype::Freetype)
