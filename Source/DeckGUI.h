#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "DeckPlayer.h"
#include "WaveformDisplay.h"
#include "PlaylistDragPayload.h"

// UI de um único deck: botões de transporte, sliders, waveform e arrastar-soltar.
class DeckGUI : public juce::Component,
                public juce::Button::Listener,
                public juce::Slider::Listener,
                public juce::FileDragAndDropTarget,
                public juce::DragAndDropTarget,
                public juce::Timer
{
public:
    DeckGUI(int deckId,
            DeckPlayer* playerToUse,
            juce::AudioFormatManager& formatManagerToUse,
            juce::AudioThumbnailCache& cacheToUse);
    ~DeckGUI() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Listeners
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    // Arrastar-soltar de arquivos do sistema operacional
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Arrastar-soltar de faixa vinda da playlist (drag interno do app)
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override;

    // Timer para atualizar playhead e slider de posição
    void timerCallback() override;

    // Chamado pela playlist para carregar uma faixa neste deck
    void loadFile(juce::URL audioURL);

    DeckPlayer* getPlayer() const { return player; }
    int getDeckId() const { return id; }

private:
    int id;
    DeckPlayer* player;

    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton forwardButton{ ">>" };
    juce::TextButton reverseButton{ "<<" };

    juce::Label volLabel{ "volLabel", "Vol." };
    juce::Label speedLabel{ "speedLabel", "Speed" };
    juce::Label posLabel{ "posLabel", "Pos" };

    juce::Slider volSlider{ juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                            juce::Slider::TextEntryBoxPosition::TextBoxAbove };
    juce::Slider speedSlider{ juce::Slider::SliderStyle::LinearVertical,
                              juce::Slider::TextEntryBoxPosition::TextBoxAbove };
    juce::Slider posSlider{ juce::Slider::SliderStyle::LinearHorizontal,
                             juce::Slider::TextEntryBoxPosition::NoTextBox };

    WaveformDisplay waveformDisplay;
    juce::SharedResourcePointer<juce::TooltipWindow> sharedTooltip;

    // Estado visual do drag-and-drop (para dar feedback ao usuário)
    bool dragOverActive = false;

    void loadFileURL(juce::URL audioURL);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};
