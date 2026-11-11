#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "BeatJumpComponent.h"
#include "DeckHeaderComponent.h"
#include "DeckPlayer.h"
#include "HotCuePadsComponent.h"
#include "JogWheelComponent.h"
#include "PlaylistDragPayload.h"
#include "MiniWaveformComponent.h"

//==============================================================================
// UI de um deck completo.
//
// O deck 1 e o deck 2 sao o MESMO componente, com `mirrored` invertido.
//
// ┌──────────────────────────────────────────┐
// │  DeckHeaderComponent                     │  <- info da faixa
// ├──────────────────────────────────────────┤
// │  MiniWaveformComponent                   │  <- onda estatica, agulha movel
// ├────────────────────┬─────────────────────┤
// │                    │                     │
// │  BeatJumpComponent │  JogWheelComponent  │  <- deck 1: beat jump a esquerda
// │  OU                │      + CUE          │
// │  HotCuePadsComponent│     + Play         │  <- deck 2: hot cue a direita
// │                    │                     │
// ├────────────────────┴─────────────────────┤
// │  Transport: Play, Stop, <<, >>, speed     │
// └──────────────────────────────────────────┘
//
// O `mirrored` controla:
//
// | Elemento           | mirrored = false (deck 1) | mirrored = true (deck 2) |
// |--------------------|--------------------------|--------------------------|
// | Grade de pads      | a esquerda               | a direita                |
// | Jog wheel          | a direita                | a esquerda               |
// | CUE / Play         | a esquerda do jog        | a direita do jog         |
// | Rótulos de EQ      | a direita                | a esquerda               |
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §7.
class DeckComponent : public juce::Component,
                      public juce::Button::Listener,
                      public juce::Slider::Listener,
                      public juce::FileDragAndDropTarget,
                      public juce::DragAndDropTarget,
                      public juce::Timer
{
public:
    // `useHotCuePads` = true mostra a grade de Hot Cue (deck 2);
    // false mostra a grade de Beat Jump (deck 1).
    DeckComponent (int deckId,
                   bool mirroredToUse,
                   bool useHotCuePadsToUse,
                   DeckPlayer* playerToUse,
                   juce::AudioFormatManager& formatManagerToUse,
                   juce::AudioThumbnailCache& cacheToUse);
    ~DeckComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Listeners
    void buttonClicked (juce::Button* button) override;
    void sliderValueChanged (juce::Slider* slider) override;

    // Arrastar-soltar de arquivos do sistema operacional
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    // Arrastar-soltar de faixa vinda da playlist (drag interno do app)
    bool isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragEnter (const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragExit (const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped (const juce::DragAndDropTarget::SourceDetails& details) override;

    // Timer para atualizar playhead e slider de posicao
    void timerCallback() override;

    // Chamado pela playlist para carregar uma faixa neste deck.
    void loadFile(juce::URL audioURL, double bpm);
    void loadFile(juce::URL audioURL, double bpm,
                  const juce::String& title, const juce::String& artist);

    DeckPlayer* getPlayer() const { return player; }
    int getDeckId() const { return id; }

private:
    void loadFileURL (juce::URL audioURL, const juce::String& title = {},
                      const juce::String& artist = {});
    void updateHeaderTimes();

    int id;
    bool mirrored;
    bool useHotCuePads;

    DeckPlayer* player;

    DeckHeaderComponent header;
    MiniWaveformComponent waveformDisplay;
    JogWheelComponent jogWheel;
    BeatJumpComponent beatJump;
    HotCuePadsComponent hotCuePads;

    // Transport
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton forwardButton { ">>" };
    juce::TextButton reverseButton { "<<" };
    juce::TextButton cueButton { "CUE" };

    juce::Slider speedSlider { juce::Slider::LinearVertical, juce::Slider::NoTextBox };

    // Estado visual do drag-and-drop (para dar feedback ao usuario)
    bool dragOverActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckComponent)
};
