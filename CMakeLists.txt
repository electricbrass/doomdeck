cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD 451f2fe2-a8a2-47c3-bc32-94786d8fc91b)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(
  MyDoomLauncher
  VERSION 0.1.0
  DESCRIPTION "A Linux-first Doom launcher"
)

add_library(mydoomlauncher_lib STATIC)

target_sources(mydoomlauncher_lib
  PUBLIC FILE_SET CXX_MODULES FILES
    src/foo.cpp
    src/desktopfileloader.cpp
    src/stringutils.cpp
)

add_executable(mydoomlauncher)

target_sources(mydoomlauncher
  PRIVATE
    src/main.cpp
    src/RmlUi_Backend_SDL_SDLrenderer.cpp
)

target_link_libraries(mydoomlauncher PRIVATE mydoomlauncher_lib)

# TODO: add options for sanitizers: ASan, UBSan, and others
# probably no need for TSan here

option(ENABLE_ALL_WARNINGS "Enable all compiler warnings" OFF)
option(ENABLE_WERROR "Treat compiler warnings as errors" OFF)
option(ENABLE_UBSAN "Enable undefined behavior sanitizer" OFF)
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# TODO: add options for static linking of standard library, SDL, and freetype
# Other dependencies will probably just do as always static

# TODO: add options for dev, ci, and user builds
# dev and ci should have pretty much maximal warnings and errors
# user should just have stuff we can guarantee
# like class-memaccess or old-style-cast
# target_compile_options(mydoomlauncher PRIVATE "-Werror")

include(CheckCXXCompilerFlag)

function(checked_add_compile_flags target)
  foreach(flag IN LISTS ARGN)
    string(MAKE_C_IDENTIFIER "${flag}" flag_id)
    set(var_name "flag_supported${flag_id}")
    check_cxx_compiler_flag("${flag}" ${var_name})
    if(${var_name})
      target_compile_options(${target} PRIVATE "${flag}")
    endif()
  endforeach()
endfunction()

function(checked_add_compile_flag_alternative target test fallback)
  string(MAKE_C_IDENTIFIER "${test}" flag_id)
  set(var_name "flag_supported${flag_id}")
  check_cxx_compiler_flag("${test}" ${var_name})
  if(${var_name})
    target_compile_options(${target} PRIVATE "${test}")
  else()
    target_compile_options(${target} PRIVATE "${fallback}")
  endif()
endfunction()

function(configure_target target)
  set_target_properties(${target}
    PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED ON
      CXX_MODULE_STD ON
  )

  # Enable some errors that should always be used
  checked_add_compile_flags(${target}
    "-Werror=old-style-cast"
    "-Werror=class-memaccess"
    "-Werror=duplicated-cond"
    "-Werror=duplicated-branches"
    "-Werror=init-self"
    "-Werror=no-self-move" # gcc
    "-Werror=self-move"    # clang
    "-Werror=non-virtual-dtor"
    "-Werror=cast-user-defined"
    "-Werror=switch"
  )

  checked_add_compile_flag_alternative(${target}
    "-Werror=implicit-fallthrough=5"
    "-Werror=implicit-fallthrough"
  )

  # Some warnings we always want
  # but shouldn't be errors except when using -Werror
  checked_add_compile_flags(${target}
    "-Wnrvo"
    "-Wmisleading-indentation"
    "-Wparentheses"
  )

  if(ENABLE_WERROR)
    target_compile_options(${target} PRIVATE "-Werror")
  endif()

  if(ENABLE_ALL_WARNINGS)
    target_compile_options(${target}
      PRIVATE
        "-Wall"
        "-Wextra"
        # "-Wpedantic" # I think I want to be able to use GNU extensions?
        "-Wconversion"
        "-Wsign-conversion"
        "-Wshadow"
        "-Wcast-align"
    )
  endif()

  # get around an annoying clang + cmake bug
  checked_add_compile_flags(${target}
    "-Wno-error=unused-command-line-argument"
  )
endfunction()

configure_target(mydoomlauncher)
configure_target(mydoomlauncher_lib)

# TODO: make sure the correct cmake options are set for when building these
# Add cmake options to force or disable the fetching?
# CMake already has that
# with FETCHCONTENT_TRY_FIND_PACKAGE_MODE

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.12.0
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)
target_link_libraries(mydoomlauncher_lib PUBLIC nlohmann_json::nlohmann_json)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.8
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(mydoomlauncher PRIVATE SDL3::SDL3)

FetchContent_Declare(
  SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG release-3.4.4
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(SDL3_image)
target_link_libraries(mydoomlauncher PRIVATE SDL3_image::SDL3_image)

FetchContent_Declare(
  Freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
  GIT_TAG VER-2-14-3
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Freetype)
target_link_libraries(mydoomlauncher PRIVATE Freetype::Freetype)

FetchContent_Declare(
  RmlUi
  GIT_REPOSITORY https://github.com/mikke89/RmlUi
  GIT_TAG 6.2
  GIT_SHALLOW
  SYSTEM # suppress warnings on includes
)
FetchContent_MakeAvailable(RmlUi)
target_link_libraries(mydoomlauncher PRIVATE RmlUi::RmlUi)

set(RMLUI_BACKEND_PATH ${RmlUi_SOURCE_DIR}/Backends)
add_library(rmlui_backend_SDL
  OBJECT
    ${RMLUI_BACKEND_PATH}/RmlUi_Platform_SDL.cpp
    ${RMLUI_BACKEND_PATH}/RmlUi_Renderer_SDL.cpp
)
target_include_directories(rmlui_backend_SDL
  SYSTEM
  PUBLIC
    ${RMLUI_BACKEND_PATH}
)
target_compile_definitions(rmlui_backend_SDL
  PUBLIC
    RMLUI_SDL_VERSION_MAJOR=3
)
# Requires 4.2, and doesn't seem to actually help for Zed
# set_target_properties(rmlui_backend_SDL PROPERTIES
#   SKIP_LINTING ON
# )
target_compile_options(rmlui_backend_SDL PRIVATE "-w")
target_link_libraries(rmlui_backend_SDL PUBLIC RmlUi::RmlUi)
target_link_libraries(mydoomlauncher PRIVATE rmlui_backend_SDL)

# TODO: Figure out how install works
# need to install binary, icons, metainfo, desktop file

if(BUILD_TESTS)
  find_package(Catch2 3 CONFIG REQUIRED)

  add_executable(unittest
    test/foo.cpp
  )

  configure_target(unittest)

  target_link_libraries(unittest PRIVATE mydoomlauncher_lib)
  target_link_libraries(unittest PRIVATE Catch2::Catch2WithMain)

  include(CTest)
  include(Catch)
  catch_discover_tests(unittest)
endif()
