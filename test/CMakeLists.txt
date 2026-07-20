find_package(Catch2 3 CONFIG REQUIRED)

add_executable(unittest
  directories.cpp
  utils.cpp
  subprocess.cpp
)

configure_target(unittest)

target_link_libraries(unittest PRIVATE ${APP_LIB})
target_link_libraries(unittest PRIVATE Catch2::Catch2WithMain)

# include(CTest)
include(Catch)
catch_discover_tests(unittest ADD_TAGS_AS_LABELS)
