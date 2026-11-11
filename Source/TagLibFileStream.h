#pragma once

#include <juce_core/juce_core.h>

// Forward declaration: o tipo vem do TagLib.
namespace TagLib
{
    class IOStream;
}

//==============================================================================
// Adaptador de stream para o TagLib.
//
// MOTIVACAO: o TagLib abre arquivos com `fopen(const char*)`, que no Windows
// usa a codepage ANSI do sistema — um caminho com acento ("Músicas") falha
// silenciosamente. Este adaptador abre o arquivo pela API de caminho LARGO
// do Windows (`_wfopen`), garantindo que caminhos Unicode funcionem.
//
// O TagLib NAO toma posse do stream: quem cria e deleta.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §8.3.
class TagLibFileStream
{
public:
    explicit TagLibFileStream (const juce::File& fileToUse, bool readOnlyToUse);
    ~TagLibFileStream();

    // Verdadeiro se o arquivo foi aberto.
    bool isOpen() const { return handle != nullptr; }

    // Acesso ao stream para o TagLib.
    TagLib::IOStream* getStream() { return stream; }

    // Nome do arquivo, como o TagLib espera (const char*).
    const char* getName() const { return name.toRawUTF8(); }

private:
    juce::File file;
    bool readOnly;
    juce::String name;

    void* handle = nullptr;             // FILE*
    TagLib::IOStream* stream = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TagLibFileStream)
};
