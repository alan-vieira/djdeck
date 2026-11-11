#pragma once

#include <juce_core/juce_core.h>
#include "Song.h"

//==============================================================================
// Persistencia da biblioteca de faixas.
//
// A biblioteca e gravada em XML no diretorio de dados da aplicacao, para que
// as faixas voltem quando o app reabre.
//
// O formato e XML (e nao JSON) porque e o mesmo mecanismo que o proprio JUCE
// usa para settings, e porque `juce::var` perdeu os tipos de objeto no
// JUCE 9 — o `juce::JSON` ficou so com metodos estaticos de parse/toString.
//
// Ver docs/ROADMAP.md — "Biblioteca para metadados".
class LibraryStore
{
public:
    // Caminho do arquivo onde a biblioteca e gravada.
    static juce::File getLibraryFile();

    // Grava a lista de faixas. Retorna true em caso de sucesso.
    static bool save (const juce::Array<Song>& songs);

    // Le a lista de faixas gravada.
    //
    // Faixas cujo arquivo nao existe mais sao ignoradas — o usuario pode ter
    // movido ou apagado a musica. As metadados sao relidas do arquivo, para
    // que uma edicao feita fora do app apareca.
    static juce::Array<Song> load();
};
