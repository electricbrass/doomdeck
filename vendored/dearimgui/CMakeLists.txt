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
  PUBLIC FILE_SET HEADERS BASE_DIRS
    include
    src/misc/freetype
)

target_compile_definitions(dearimgui
  PUBLIC
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_ENABLE_FREETYPE
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
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)
set(FT_DISABLE_HVF ON)
FetchContent_MakeAvailable(Freetype)
if(TARGET freetype-interface AND NOT TARGET Freetype::Freetype)
  add_library(Freetype::Freetype ALIAS freetype-interface)
endif()
target_link_libraries(dearimgui PRIVATE Freetype::Freetype)
