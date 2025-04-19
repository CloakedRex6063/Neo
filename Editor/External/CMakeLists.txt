FetchContent_Declare(
        ImGUI
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.91.9b-docking
)
FetchContent_MakeAvailable(ImGUI)
FILE(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
set(IMGUI_SOURCES 
        ${IMGUI_SOURCES}
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui PUBLIC glfw dxgi.lib d3d12.lib)
target_link_libraries(Editor PUBLIC imgui)
target_include_directories(Editor PUBLIC MaterialIcons)