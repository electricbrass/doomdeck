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

include(FetchContent)
include(CTest)
include(GetGitInfo)

string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)
set(APP_ID "net.electricbrass.${PROJECT_NAME_LOWER}")

set(APP_EXE "${PROJECT_NAME_LOWER}")
set(APP_LIB "${PROJECT_NAME_LOWER}_lib")

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
    src/flatpakutils.cpp
    src/shellutils.cpp
    src/appinfo.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/appinfo_config.cpp
)

# Images for #embed
if (CMAKE_VERSION VERSION_EQUAL "4.4.0")
  # Workaround for a bug in 4.4.0
  target_compile_options(${APP_LIB} PUBLIC
    "--embed-dir=${CMAKE_CURRENT_SOURCE_DIR}/res"
  )
else()
  target_compile_options(${APP_LIB} PRIVATE
    "--embed-dir=${CMAKE_CURRENT_SOURCE_DIR}/res"
  )
endif()

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
option(ENABLE_LTO "Enable link-time optimization" OFF)
option(STATIC_CXX_RUNTIME "Statically link the C++ runtime" OFF)
option(INSTALL_LEGACY_METADATA "Install AppStream metadata with legacy .appdata.xml extension" OFF)
option(USE_INTERNAL_SQLITE "Use vendored SQLite instead of system package" OFF)

if (STATIC_CXX_RUNTIME)
  if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR
      "Static C++ runtime is only supported if compiling with GCC")
  else()
    target_link_options(${APP_EXE} PRIVATE -static-libstdc++ -static-libgcc)
  endif()
endif()

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
  # and a clang + libstdc++ bug
  checked_add_compile_flags(${target}
    "-Wno-error=unused-command-line-argument"
    "-Wno-error=reserved-module-identifier"
  )
endfunction()

configure_target(${APP_EXE})
configure_target(${APP_LIB})

# Enable support for std::stacktrace
# Currently this forces building with libstdc++
target_link_libraries(${APP_LIB} PUBLIC stdc++exp)

# For anything we're building, it might as well be static
set(BUILD_SHARED_LIBS OFF)

FetchContent_Declare(
  zlib-ng
  GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng
  GIT_TAG 2.3.3
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
# We don't want to build zlib-ng's tests
# but it uses the same BUILD_TESTING variable
# as CTest
set(_save_build_testing ${BUILD_TESTING})
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(zlib-ng)
set(BUILD_TESTING ${_save_build_testing})
unset(_save_build_testing)
target_link_libraries(${APP_LIB} PUBLIC zlib-ng::zlib)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG v8.0.0
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(glaze)
target_link_libraries(${APP_LIB} PUBLIC glaze::glaze)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.14
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
set(SDL_X11_XTEST OFF)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(${APP_EXE} PRIVATE SDL3::SDL3)

FetchContent_Declare(
  libsl3
  GIT_REPOSITORY https://github.com/a4z/libsl3
  GIT_TAG v1.2.53003
  GIT_SHALLOW
  FIND_PACKAGE_ARGS
)
set(sl3_BUILD_TESTING OFF)
set(sl3_BUILD_DOCS OFF)
set(sl3_USE_COMMON_COMPILER_WARNINGS OFF)
set(sl3_USE_INTERNAL_SQLITE3 ${USE_INTERNAL_SQLITE})
FetchContent_MakeAvailable(libsl3)
target_link_libraries(${APP_LIB} PUBLIC sl3)

add_subdirectory(vendored SYSTEM)
target_link_libraries(${APP_EXE} PRIVATE dearimgui)
target_link_libraries(${APP_EXE} PRIVATE stb)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

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
if(INSTALL_LEGACY_METAINFO)
  install(
    FILES packaging/${APP_ID}.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
    RENAME ${APP_ID}.appdata.xml
  )
endif()

install(
  FILES COPYING
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME_LOWER}
)

# TODO: figure out CPack
