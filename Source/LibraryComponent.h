#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LibraryTreeModel.h"
#include "PlaylistComponent.h"

//==============================================================================
// Biblioteca: arvore de navegacao a esquerda (~30%) + lista de faixas a
// direita (~70%).
//
// ┌──────────────┬──────────────────────────────────────┐
// │ Collection   │  Título  Artista  BPM  Rating  Time   │
// │ ├ Playlists  │  ...                                  │
// │ ├ Explorer   │  ...                                  │
// │ ├ Devices    │                                      │
// │ ├ Histories  │                                      │
// │ └ Recordings │                                      │
// └──────────────┴──────────────────────────────────────┘
//
// O divisor redimensionavel usa juce::StretchableLayoutManager — o
// juce::SplitterComponent nao existe no JUCE 9.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §8.
class LibraryComponent : public juce::Component
{
public:
    // O formatManager e necessario para a deteccao de BPM por analise de
    // audio ao importar faixas (ver PlaylistComponent).
    LibraryComponent (juce::AudioFormatManager& formatManagerToUse);
    ~LibraryComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // A lista de faixas, para que o MainComponent possa ligar o duplo-clique
    // ao carregamento no deck.
    PlaylistComponent& getTrackList() { return *trackList; }

private:
    void buildTree();

    juce::TreeView treeView;

    // O TreeView NAO tem posse do root — guardamos como member.
    std::unique_ptr<LibraryTreeModel> treeRoot;

    // Declarado antes de trackList porque o construtor da playlist recebe a
    // referencia ao formatManager.
    juce::AudioFormatManager& formatManager;

    std::unique_ptr<PlaylistComponent> trackList;

    juce::StretchableLayoutManager layoutManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibraryComponent)
};
