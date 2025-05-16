FetchContent_Declare(
        GLFW
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.4
)
FetchContent_MakeAvailable(GLFW)
target_link_libraries(Engine PUBLIC glfw)

FetchContent_Declare(
        MAGIC_ENUM
        GIT_REPOSITORY https://github.com/Neargye/magic_enum
        GIT_TAG v0.9.7
)
FetchContent_MakeAvailable(MAGIC_ENUM)
target_link_libraries(Engine PUBLIC magic_enum::magic_enum)

FetchContent_Declare(
        FASTGLTF
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG  v0.8.0
)
FetchContent_MakeAvailable(FASTGLTF)
target_link_libraries(Engine PUBLIC fastgltf::fastgltf)

FetchContent_Declare(
        SPDLOG
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG v1.15.1
)
FetchContent_MakeAvailable(SPDLOG)
target_link_libraries(Engine PUBLIC spdlog::spdlog)

FetchContent_Declare(
        GLM
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 2d4c4b4
)
FetchContent_MakeAvailable(GLM)
target_link_libraries(Engine PUBLIC glm::glm)

FetchContent_Declare(
        ENTT
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG  v3.15.0
)
FetchContent_MakeAvailable(ENTT)
target_link_libraries(Engine PUBLIC EnTT::EnTT)

FetchContent_Declare(
        DX12Headers
        GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers
        GIT_TAG v1.615.0
)
FetchContent_MakeAvailable(DX12Headers)
target_link_libraries(Engine PUBLIC Microsoft::DirectX-Headers)

set(UUID_SYSTEM_GENERATOR ON)
FetchContent_Declare(
        UUID
        GIT_REPOSITORY https://github.com/mariusbancila/stduuid
        GIT_TAG v1.2.3 
)
FetchContent_MakeAvailable(UUID)
target_link_libraries(Engine PUBLIC stduuid)

add_library(STB STB/stb_image.cpp)
target_include_directories(STB PUBLIC STB)
target_link_libraries(Engine PUBLIC STB)

add_library(DXIL INTERFACE)
target_include_directories(DXIL INTERFACE DXIL/inc)
target_link_libraries(Engine PUBLIC DXIL)
target_link_libraries(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DXIL/lib/dxcompiler.lib)
target_link_libraries(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DXIL/lib/dxil.lib)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Mono/Include)
target_link_libraries(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Mono/Lib/mono-2.0-sgen.lib)

FetchContent_Declare(
        ZPP_BITS
        GIT_REPOSITORY https://github.com/eyalz800/zpp_bits
        GIT_TAG v4.5
)
FetchContent_MakeAvailable(ZPP_BITS)
add_library(ZPP INTERFACE)
target_include_directories(ZPP INTERFACE ${zpp_bits_SOURCE_DIR})
target_link_libraries(Engine PUBLIC ZPP)

FetchContent_Declare(
        GLAZE
        GIT_REPOSITORY https://github.com/stephenberry/glaze
        GIT_TAG v5.2.0)
FetchContent_MakeAvailable(GLAZE)
target_link_libraries(Engine PUBLIC glaze::glaze)