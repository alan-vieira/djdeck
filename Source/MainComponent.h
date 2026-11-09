#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "DeckPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "MixerDeviceScanner.h"

// Forward declaration da janela de configurações (definida em SettingsDialog.h)
class SettingsDialog;

//==============================================================================
// Componente principal da aplicação.
// Gerencia o AudioAppComponent (callback de áudio), 2 decks, mixer e playlist.
//
// juce::DragAndDropContainer é um MIXIN (não deriva de Component), então pode
// conviver com juce::AudioAppComponent. Ele fornece startDragging(), que é o
// que permite arrastar uma faixa da playlist até um deck.
class MainComponent : public juce::AudioAppComponent,
                      public juce::DragAndDropContainer,
                      public juce::Button::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioAppComponent
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Button::Listener
    void buttonClicked(juce::Button* button) override;

private:
    // Áudio.
    // NOTA: juce::AudioAppComponent já possui um member `AudioDeviceManager& deviceManager`
    // (protected) que é o manager realmente usado por setAudioChannels(). Declarar outro
    // member com o mesmo nome aqui esconderia o da base e criaria dois managers distintos.
    // Por isso usamos o manager herdado da base.
    juce::AudioFormatManager formatManager;

    // Cache compartilhado para miniaturas de waveform.
    // IMPORTANTE: declarado ANTES dos decks de propósito. Os membros são destruídos
    // na ordem inversa da declaração, e o destructor do AudioThumbnail (dentro do
    // WaveformDisplay de cada deck) acessa este cache. Se o cache fosse destruído
    // primeiro, teríamos um use-after-free no encerramento do app.
    juce::AudioThumbnailCache thumbnailCache{ 5 };

    MixerDeviceScanner deviceScanner;
    juce::MixerAudioSource mixerSource;

    // Decks
    std::unique_ptr<DeckPlayer> deckPlayer1;
    std::unique_ptr<DeckPlayer> deckPlayer2;
    std::unique_ptr<DeckGUI> deckGUI1;
    std::unique_ptr<DeckGUI> deckGUI2;

    // Playlist
    std::unique_ptr<PlaylistComponent> playlist;

    // Diálogo de configurações de áudio (criado sob demanda)
    std::unique_ptr<SettingsDialog> settingsDialog;

    // Botão de settings
    juce::TextButton settingsButton{ "Settings" };

    // Look and feel personalizado
    juce::LookAndFeel_V4 lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
