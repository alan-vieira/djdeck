#===============================================================================
# djdeck — Codegen do layout (JSON -> header C++)
#
# Gera Source/LayoutConfig.hpp a partir de Source/layout.json em tempo de
# build, para que os valores de UI estejam embutidos no binario sem parsing
# de JSON em runtime.
#
# Integracao em CMakeLists.txt:
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#   include(LayoutCodegen)
#   djdeck_layout_codegen()
#===============================================================================

function(djdeck_layout_codegen)
    set(options "")
    set(oneValueArgs JSON_IN HEADER_OUT)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(json_in   "${ARG_JSON_IN}"   CACHE INTERNAL "")
    set(header_out "${ARG_HEADER_OUT}" CACHE INTERNAL "")

    if (NOT json_in)
        set(json_in "${CMAKE_CURRENT_SOURCE_DIR}/Source/layout.json")
    endif()
    if (NOT header_out)
        set(header_out "${CMAKE_CURRENT_SOURCE_DIR}/Source/LayoutConfig.hpp")
    endif()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    # Custom target que regenera o header sempre que o JSON mudar.
    add_custom_target(djdeck_layout_codegen
        COMMAND ${Python3_EXECUTABLE}
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/layout_codegen.py"
                "${json_in}"
                "${header_out}"
        DEPENDS "${json_in}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/layout_codegen.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Gerando Source/LayoutConfig.hpp a partir de Source/layout.json"
        VERBATIM
    )

    # O target da aplicacao depende do codegen.
    if (TARGET djdeck)
        add_dependencies(djdeck djdeck_layout_codegen)
    endif()

    # Garante que Source/ esteja no include path (o header gerado vive la).
    if (TARGET djdeck)
        target_include_directories(djdeck PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/Source")
    endif()

    message(STATUS "djdeck: layout codegen configurado (${json_in} -> ${header_out})")
endfunction()
