FetchContent_Declare(
    SDL2
    GIT_REPOSITORY "https://github.com/SDL-mirror/SDL.git"
    GIT_TAG release-2.0.12
)
FetchContent_MakeAvailable(SDL2)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

add_library(glew STATIC IMPORTED)
set_target_properties(glew PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../extern/glew/libGLEW.a)

add_custom_target(
    copy_shader_files
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR})

add_executable(marching_cubes main.cc)
target_compile_features(marching_cubes PRIVATE cxx_std_11)
target_include_directories(marching_cubes PRIVATE ../include)
target_link_libraries(marching_cubes PRIVATE glew GLU GL SDL2main SDL2-static)

add_dependencies(marching_cubes copy_shader_files)
