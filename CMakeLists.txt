cmake_minimum_required(VERSION 3.20)
project(auto_path_gen LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # force static build
add_subdirectory(extern/raylib)

add_executable(auto_path_gen
    src/main.cpp
    src/raygui_impl.cpp
    src/utils.cpp
    src/types.cpp
    src/ui_elements.cpp
    src/app_state.cpp
    src/codegen.cpp
    src/sidebar.cpp
    src/popups.cpp
    src/field_renderer.cpp
)
#raygui
target_include_directories(${PROJECT_NAME} PUBLIC
    extern/raygui/src   # include Raygui headers
)
# Your executable



# Link raylib statically
target_link_libraries(auto_path_gen PRIVATE raylib)

if(MINGW)
    target_compile_definitions(auto_path_gen PRIVATE "-DPLATFORM_DESKTOP" "-DGRAPHICS_API_OPENGL_33")
endif()
