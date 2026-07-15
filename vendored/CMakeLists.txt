add_subdirectory(dearimgui)

add_library(stb INTERFACE)
target_sources(stb INTERFACE FILE_SET HEADERS BASE_DIRS stb)
