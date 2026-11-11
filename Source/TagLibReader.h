#pragma once

#include <juce_core/juce_core.h>
#include "TrackMetadata.h"

//==============================================================================
// Leitor e escritor de metadados, baseado no TagLib.
//
// O TagLib le e escreve tags ID3 (MP3), Vorbis Comments (OGG/FLAC/Opus),
// MP4 (M4A) e APE — decisao registrada em docs/ROADMAP.md.
//
// Os caminhos sao abertos com o ADAPTADOR TagLibFileStream, e nao com o
// `FileStream` do proprio TagLib: aquele usa `fopen(const char*)`, que no
// Windows depende da codepage ANSI e falha em caminhos com acento.
//
// As classes do TagLib nao sao thread-safe; esta classe serializa o acesso
// com um mutex, porque a leitura acontece na thread de GUI (ao adicionar
// uma faixa) e a escrita pode vir de outra.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §8.3.
class TagLibReader
{
public:
    TagLibReader();
    ~TagLibReader();

    // Le os metadados de um arquivo. Retorna um TrackMetadata com
    // `isValid = false` se o arquivo nao puder ser lido.
    TrackMetadata readMetadata (const juce::File& file) const;

    // Escreve os metadados no arquivo. Retorna true em caso de sucesso.
    //
    // Campos vazios nao sobrescrevem o valor existente — escrever "" apagaria
    // a tag do arquivo.
    bool writeMetadata (const juce::File& file, const TrackMetadata& metadata) const;

    // Le apenas a duracao, sem abrir todas as tags. Mais rapido para
    // listagens grandes.
    double readDurationSeconds (const juce::File& file) const;

private:
    mutable juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TagLibReader)
};
