cmake_minimum_required(VERSION 4.0)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.4.0")
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
elseif (CMAKE_VERSION VERSION_GREATER_EQUAL "4.3.0")
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
elseif (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.3")
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
else()
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
endif()
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.3.0")
  project(
    DoomDeck
    VERSION 0.1.0
    SPDX_LICENSE "GPL-3.0-or-later"
    DESCRIPTION "A Linux-first Doom launcher"
  )
else()
  project(
    DoomDeck
    VERSION 0.1.0
    DESCRIPTION "A Linux-first Doom launcher"
  )
endif()

string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)
set(APP_ID "net.electricbrass.${PROJECT_NAME_LOWER}")

set(APP_EXE "${PROJECT_NAME_LOWER}")
set(APP_LIB "${PROJECT_NAME_LOWER}_lib")

include(GetGitInfo)
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)
configure_file(
  src/appinfo_config.cpp.in
  appinfo_config.cpp
)

add_library(${APP_LIB} STATIC)

target_sources(${APP_LIB}
  PUBLIC FILE_SET CXX_MODULES FILES
    src/desktopfileloader.cpp
    src/stringutils.cpp
    src/errors.cpp
    src/config.cpp
    src/directories.cpp
    src/icons.cpp
    src/subprocess.cpp
    src/utils.cpp
    src/appinfo.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/appinfo_config.cpp
)

# Images for #embed
target_compile_options(${APP_LIB} PRIVATE
  "--embed-dir=${CMAKE_CURRENT_SOURCE_DIR}/res"
)

add_executable(${APP_EXE})

target_sources(${APP_EXE}
  PRIVATE
    src/main.cpp
  PRIVATE FILE_SET CXX_MODULES FILES
    src/ui/images.cpp
    src/ui/settings.cpp
    src/ui/types.cpp
    src/ui/window.cpp
    src/appstate.cpp
)

target_link_libraries(${APP_EXE} PRIVATE ${APP_LIB})

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
# target_compile_options(doomdeck PRIVATE "-Werror")

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
    "-Werror=zero-as-null-pointer-constant"
    "-Werror=return-type"
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

configure_target(${APP_EXE})
configure_target(${APP_LIB})

# Enable support for std::stacktrace
# Currently this forces building with libstdc++
target_link_libraries(${APP_LIB} PUBLIC stdc++exp)

# TODO: make sure the correct cmake options are set for when building these
# Add cmake options to force or disable the fetching?
# CMake already has that
# with FETCHCONTENT_TRY_FIND_PACKAGE_MODE

include(FetchContent)

FetchContent_Declare(
  zlib-ng
  GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng
  GIT_TAG 2.3.3
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(zlib-ng)
# TODO: static or shared for this?
target_link_libraries(${APP_LIB} PUBLIC zlib-ng::zlib)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.12.0
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)
target_link_libraries(${APP_LIB} PUBLIC nlohmann_json::nlohmann_json)

# TODO: maybe don't bother with fetchcontent here
# building sdl has lots of options and a lot of dev dependencies
# so it's probably better to just use whatever the system provides
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.12
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(${APP_EXE} PRIVATE SDL3::SDL3)

add_subdirectory(vendored SYSTEM)
target_link_libraries(${APP_EXE} PRIVATE dearimgui)
target_link_libraries(${APP_EXE} PRIVATE stb)

include(GNUInstallDirs)
configure_file(packaging/${APP_ID}.desktop.in ${APP_ID}.desktop)
install(TARGETS ${APP_EXE})
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${APP_ID}.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(
  FILES res/icon/${PROJECT_NAME_LOWER}.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
  RENAME ${APP_ID}.png
)
install(
  FILES packaging/${APP_ID}.metainfo.xml
  DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)
install(
  FILES COPYING
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME_LOWER}
)

# TODO: figure out CPack

if(BUILD_TESTS)
  find_package(Catch2 3 CONFIG REQUIRED)

  add_executable(unittest
    test/directories.cpp
  )

  configure_target(unittest)

  target_link_libraries(unittest PRIVATE ${APP_LIB})
  target_link_libraries(unittest PRIVATE Catch2::Catch2WithMain)

  include(CTest)
  include(Catch)
  catch_discover_tests(unittest ADD_TAGS_AS_LABELS)
endif()
