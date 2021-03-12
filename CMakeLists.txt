cmake_minimum_required(VERSION 3.15)
project(mipt-cg)

set(CMAKE_CXX_STANDARD 17)

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_CXX_COMPILER)
cmake_print_variables(EMSCRIPTEN)
cmake_print_variables(CMAKE_EXECUTABLE_SUFFIX)

if (NOT EMSCRIPTEN)
    add_subdirectory(external/glfw)
    add_subdirectory(external/glew-cmake)
#    find_package(glfw3 REQUIRED)
#    find_package(GLEW REQUIRED)
endif()

add_subdirectory(external/glm)

if (NOT EMSCRIPTEN)
    add_executable(main main.cpp)
    target_link_libraries(main glm)

    target_link_libraries(main glfw libglew_static GL)
else()
    add_executable(main main.cpp shaders/vertex.glsl shaders/fragment.glsl)
    target_link_libraries(main glm)

    # target_compile_options(
    #     main
    #     PRIVATE
    #     -g4
    # )

    target_link_options(main PRIVATE
        "SHELL:
        -s USE_GLFW=3
        --preload-file ${CMAKE_SOURCE_DIR}/shaders@./shaders
        --preload-file ${CMAKE_SOURCE_DIR}/data@./data
        --shell-file /usr/lib/emscripten/src/shell_minimal.html
        " # SHELL: is used to avoid deduplication of options (cmake does it by default)
    ) # TODO: add dependency tracking

    # -g4
    # --source-map-base http://localhost:8000/

    set_target_properties(main PROPERTIES SUFFIX ".html")
    #set_property(TARGET main APPEND PROPERTY LINK_DEPENDS ./shaders/fragment.glsl ./shaders/vertex.glsl)

    #cmake_print_properties(TARGETS main PROPERTIES LINK_DEPENDS)

    #add_custom_command(OUTPUT ./shaders/vertex.glsl)
    #add_custom_target(shaders DEPENDS ./shaders/vertex.glsl ./shaders/fragment.glsl)
    #add_dependencies(main shaders)
endif()
