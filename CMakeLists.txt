cmake_minimum_required(VERSION 3.25)

project(djdeck VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Runtime estático do MSVC (/MT em vez de /MD) - elimina dependência de DLLs
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Codificação dos fontes.
#
# Sem /utf-8, o MSVC assume a codepage do sistema (Windows-1252) ao ler os
# fontes, e strings acentuadas viram mojibake na interface: o cabeçalho
# "Duração" aparecia com os acentos quebrados.
#
# /utf-8 faz o compilador ler E gravar os fontes como UTF-8, independente da
# configuração regional da máquina — o que mantém a acentuação correta em
# qualquer computador. No GCC/Clang, as flags equivalentes já são o padrão,
# mas explicitá-las não causa mal e documenta a intenção.
if (MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# JUCE via FetchContent (baixa automaticamente, não precisa instalar manualmente)
set(FETCHCONTENT_QUIET OFF)
include(FetchContent)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 7.0.3 GLOBAL)

FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(djdeck)

file(GLOB_RECURSE SourceFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

# Arquivos de diagnostico ficam em Scratch/ e nao devem ser compilados
list(FILTER SourceFiles EXCLUDE REGEX "/Scratch/")

target_sources("${PROJECT_NAME}" PRIVATE ${SourceFiles})

target_link_libraries("${PROJECT_NAME}" PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
