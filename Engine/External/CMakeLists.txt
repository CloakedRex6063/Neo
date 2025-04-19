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
        SimdJson
        GIT_REPOSITORY https://github.com/simdjson/simdjson
        GIT_TAG v3.12.3
)
FetchContent_MakeAvailable(SimdJson)
target_link_libraries(Engine PUBLIC simdjson::simdjson)

FetchContent_Declare(
        Json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(Json)
target_link_libraries(Engine PUBLIC nlohmann_json)

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

target_include_directories(Engine PUBLIC DXIL/inc)
target_link_libraries(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DXIL/lib/dxcompiler.lib)
target_link_libraries(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DXIL/lib/dxil.lib)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Mono/Include)
target_link_libraries(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Mono/Lib/mono-2.0-sgen.lib)
