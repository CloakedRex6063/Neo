FILE(GLOB_RECURSE EDITOR_SOURCES Source/*.cpp)
add_executable(Editor ${EDITOR_SOURCES})
target_include_directories(Editor PUBLIC Include)
target_link_libraries(Editor PUBLIC Engine)

add_subdirectory(External)

add_custom_target(COPY_ASSETS ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Editor/Assets"
        "$<TARGET_FILE_DIR:Editor>/Assets"
)
add_dependencies(Editor COPY_ASSETS)
add_custom_target(COPY_DLL ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Editor>/mono/4.5"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/Engine/External/Mono/DLL/mono-2.0-sgen.dll"
        "$<TARGET_FILE_DIR:Editor>"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/Engine/External/Mono/DLL/mscorlib.dll"
        "$<TARGET_FILE_DIR:Editor>/mono/4.5/"
        COMMENT "Copying dlls to target directory"
)
add_dependencies(Editor COPY_DLL)

add_custom_target(COPY_DXIL ALL
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/Engine/External/DXIL/bin/dxcompiler.dll"
        "$<TARGET_FILE_DIR:Editor>"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/Engine/External/DXIL/bin/dxil.dll"
        "$<TARGET_FILE_DIR:Editor>"
        COMMENT "Copying DXIL to target directory"
)
add_dependencies(Editor COPY_DXIL)

add_custom_target(GEN_SHADERS ALL
        COMMAND python ${CMAKE_SOURCE_DIR}/Scripts/Python/GenerateShaders.py
        -dxc_path "${CMAKE_SOURCE_DIR}/Engine/External/DXIL/bin/dxc.exe"
        -source_folder "${CMAKE_SOURCE_DIR}/Engine/Shaders/"
        -output_folder "${CMAKE_CURRENT_BINARY_DIR}/Shaders"
        COMMENT "Generating Shaders"
)
add_dependencies(Editor GEN_SHADERS)

if (WITH_SANDBOX)
    add_custom_target(COPY_PROJECT
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Sandbox/"
        "$<TARGET_FILE_DIR:Editor>/Sandbox/"
        COMMENT "Copying the game project"
    )
    add_dependencies(Editor COPY_PROJECT)
endif ()