#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
// Payload transportado durante o drag-and-drop interno da aplicação.
//
// A JUCE transporta o payload do drag dentro de um juce::var. Como o var não
// oferece templates is<T>()/get<T>() genéricos no JUCE 9, usamos um
// ReferenceCountedObject próprio: o var o armazena com contagem de referência
// e o getObject() permite recuperar o ponteiro para um dynamic_cast.
//
// Usar um payload TIPADO (em vez de uma String crua) é importante porque
// distingue o drag interno "playlist -> deck" do drop de arquivos vindo do
// sistema operacional, que trafega por um caminho completamente diferente
// (FileDragAndDropTarget::filesDropped).
//
// O token "type" evita que um drop de texto externo (ex.: arrastar uma URL do
// navegador para dentro da janela) seja interpretado como uma faixa.
class PlaylistDragPayload : public juce::ReferenceCountedObject
{
public:
    static constexpr const char* typeToken = "djdeck/playlistTrack";

    explicit PlaylistDragPayload (juce::File fileToUse)
        : file (std::move (fileToUse))
    {
    }

    juce::File file;

    static bool isPayload (const juce::var& value)
    {
        if (value.isVoid())
            return false;

        return dynamic_cast<PlaylistDragPayload*> (value.getObject()) != nullptr;
    }

    static PlaylistDragPayload* getFrom (const juce::var& value)
    {
        return dynamic_cast<PlaylistDragPayload*> (value.getObject());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistDragPayload)
};
