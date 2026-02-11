cmake_minimum_required(VERSION 3.20)
project(auto_path_gen LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # force static build

# ---- Dear ImGui ----
add_library(imgui STATIC
    ${CMAKE_CURRENT_LIST_DIR}/extern/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/extern/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/extern/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_LIST_DIR}/extern/imgui/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/extern/imgui
)

target_compile_features(imgui PUBLIC cxx_std_17)

# ---- raylib ----
add_subdirectory(extern/raylib)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_executable(auto_path_gen WIN32
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
        app.rc
    )
else()
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
        app.rc
    )
endif()

if(EMSCRIPTEN)
    target_link_options(auto_path_gen PRIVATE
        "-sASYNCIFY"
        "-sASYNCIFY_IMPORTS=emscripten_sleep"
    )
    set_target_properties(auto_path_gen PROPERTIES
        SUFFIX ".html"
    )
endif()

# raygui
target_include_directories(${PROJECT_NAME} PUBLIC
    extern/raygui/src
)

# Link raylib statically
target_link_libraries(auto_path_gen PRIVATE raylib)

# Link ImGui
target_link_libraries(auto_path_gen PRIVATE imgui)

if(MINGW)
    target_compile_definitions(auto_path_gen PRIVATE "-DPLATFORM_DESKTOP" "-DGRAPHICS_API_OPENGL_33")
endif()
