cmake_minimum_required(VERSION 3.25)

project(djdeck VERSION 0.3.0)

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
    GIT_TAG 9.0.5          # tag versionada: build reprodutivel
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 7.0.3 GLOBAL)

FetchContent_MakeAvailable(JUCE)

#===============================================================================
# TagLib — leitura e escrita de metadados de audio.
#
# Decisao registrada em docs/ROADMAP.md. Le e escreve tags ID3 (MP3),
# Vorbis Comments (OGG/FLAC/Opus), MP4 (M4A) e APE.
#
# Configuracao:
#   - Build estatico (BUILD_SHARED_LIBS OFF) e com o mesmo runtime /MT do app.
#     Sem ENABLE_STATIC_RUNTIME, o TagLib seria compilado com /MD e o link
#     falharia com LNK2038 (runtime mismatch) contra o /MT do djdeck.
#   - WITH_ZLIB OFF: zlib so e necessario para descomprimir DADOS de audio.
#     Os metadados ficam em blocos nao comprimidos (ID3, Vorbis Comments,
#     FLAC METADATA_BLOCK, atoms MP4). Desligar remove a dependencia e
#     elimina o risco de linkar uma zlib /MD contra o app /MT.
#   - Formatos exoticos desligados (ASF, MOD, Shorten, TrueAudio, DSF,
#     Matroska): nao usados pelo djdeck e cada um adiciona tempo de build.
#   - BUILD_TESTING OFF: o TagLib procuraria CppUnit, que nao existe aqui.
#   - BUILD_BINDINGS OFF: as bindings C do TagLib nao sao usadas.
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_BINDINGS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
set(WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(WITH_ASF OFF CACHE BOOL "" FORCE)
set(WITH_MOD OFF CACHE BOOL "" FORCE)
set(WITH_SHORTEN OFF CACHE BOOL "" FORCE)
set(WITH_TRUEAUDIO OFF CACHE BOOL "" FORCE)
set(WITH_DSF OFF CACHE BOOL "" FORCE)
set(WITH_MATROSKA OFF CACHE BOOL "" FORCE)

# utfcpp — dependencia header-only do TagLib (UTF-8 <-> UTF-16).
#
# O TagLib normalmente usa o proprio submodule 3rdparty/utfcpp, mas
# FetchContent com GIT_SHALLOW nao popula submodules. Buscamos o utfcpp
# separadamente e apontamos a variavel de cache que o TagLib procura via
# find_path.
FetchContent_Declare(
    UTF8CPP
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
    GIT_TAG v4.2.1
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(UTF8CPP)

# O nome das variaveis de FetchContent e MINUSCULO: `utf8cpp_SOURCE_DIR`.
set(utf8cpp_INCLUDE_DIR "${utf8cpp_SOURCE_DIR}/source"
    CACHE PATH "Include directory for utfcpp")

FetchContent_Declare(
    TAGLIB
    GIT_REPOSITORY https://github.com/taglib/taglib.git
    GIT_TAG v2.3.2          # tag versionada: build reprodutivel
    GIT_SHALLOW ON)

FetchContent_MakeAvailable(TAGLIB)

juce_add_gui_app(djdeck)

file(GLOB_RECURSE SourceFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

# Arquivos de diagnostico ficam em Scratch/ e nao devem ser compilados
list(FILTER SourceFiles EXCLUDE REGEX "/Scratch/")

target_sources("${PROJECT_NAME}" PRIVATE ${SourceFiles})

# O target do TagLib chama-se `tag` (o nome `TagLib::taglib` so existe no
# pacote instalado). O target `tag` expoe apenas os include dirs de
# INSTALL_INTERFACE, entao os diretorios de include do codigo-fonte e do
# build precisam ser declarados aqui para o codigo do djdeck.
#
# Os headers do TagLib incluem uns aos outros por nome curto ("tfile.h",
# "taglib_export.h"), e cada um vive no seu subdiretorio (toolkit, mpeg,
# mpeg/id3v2, ogg, flac, mp4, ...). Por isso precisamos de TODOS esses
# diretorios no include path, nao apenas da raiz.
#
# A raiz `taglib/` tambem e necessaria: e onde esta o `taglib_export.h`, e o
# target `tag` do proprio TagLib a recebe implicitamente (CMake sempre
# adiciona o source dir e o binary dir do target ao include path) — o nosso
# target, que vive na raiz do projeto, nao recebe.
set(TAGLIB_INCLUDE_DIRS
    ${taglib_SOURCE_DIR}/taglib/toolkit
    ${taglib_SOURCE_DIR}/taglib/mpeg
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2/frames
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v1
    ${taglib_SOURCE_DIR}/taglib/ogg
    ${taglib_SOURCE_DIR}/taglib/ogg/flac
    ${taglib_SOURCE_DIR}/taglib/ogg/vorbis
    ${taglib_SOURCE_DIR}/taglib/ogg/speex
    ${taglib_SOURCE_DIR}/taglib/ogg/opus
    ${taglib_SOURCE_DIR}/taglib/flac
    ${taglib_SOURCE_DIR}/taglib/ape
    ${taglib_SOURCE_DIR}/taglib/mpc
    ${taglib_SOURCE_DIR}/taglib/wavpack
    ${taglib_SOURCE_DIR}/taglib/mp4
    ${taglib_SOURCE_DIR}/taglib/riff
    ${taglib_SOURCE_DIR}/taglib/riff/aiff
    ${taglib_SOURCE_DIR}/taglib/riff/wav)

target_include_directories("${PROJECT_NAME}" PRIVATE
    ${taglib_SOURCE_DIR}          # taglib/fileref.h
    ${taglib_SOURCE_DIR}/taglib   # taglib_export.h
    ${taglib_BINARY_DIR}          # config.h e taglib_config.h gerados
    ${utf8cpp_SOURCE_DIR}/source  # utf8.h, exigido pelos headers do TagLib
    ${TAGLIB_INCLUDE_DIRS})       # "tfile.h", "toolkit/tstring.h", ...

target_link_libraries("${PROJECT_NAME}" PRIVATE
    tag
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

#===============================================================================
# Testes automatizados (opcional, ON por padrão)
#
# Desative com:  cmake -B Builds -DJDECK_ENABLE_TESTS=OFF ...
# Ative com:     cmake -B Builds -DJDECK_ENABLE_TESTS=ON ...
# Execute:       cmake --build Builds --config Debug --target djdeck_tests
#                Builds/djdeck_artefacts/Debug/djdeck_tests.exe
#
# Dependencia: Catch2 v3 via FetchContent (fornece implementacoes .cpp completas).
#===============================================================================
option(DJDECK_ENABLE_TESTS "Build unit tests with Catch2" ON)

if (DJDECK_ENABLE_TESTS)
    FetchContent_Declare(
        CATCH2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(CATCH2)

    add_executable(djdeck_tests
        # main.cpp define o ponto de entrada e inicializa o JUCE dentro de
        # main() — ver o comentario la. Por isso o target usa Catch2::Catch2
        # (SEM main) e nao Catch2::Catch2WithMain.
        tests/main.cpp
        tests/test_song.cpp
        tests/test_deckplayer.cpp
        tests/test_crossfader.cpp
        tests/test_waveform.cpp

        # Implementacoes do projeto necessarias pelos testes
        Source/Song.cpp
        Source/DeckPlayer.cpp
        Source/CrossfaderComponent.cpp
    )

    target_link_libraries(djdeck_tests PRIVATE
        Catch2::Catch2
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_utils
    )

    target_include_directories(djdeck_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
    )

    target_compile_definitions(djdeck_tests PRIVATE
        DJDECK_TESTS=1
    )

    # Os testes rodam como console app (sem janela GUI).
    set_target_properties(djdeck_tests PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()
