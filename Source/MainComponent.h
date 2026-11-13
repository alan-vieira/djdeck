#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "DeckPlayer.h"
#include "DeckComponent.h"
#include "BigWaveformsComponent.h"
#include "GlobalHeaderComponent.h"
#include "LibraryComponent.h"
#include "MixerComponent.h"
#include "MixerDeviceScanner.h"
#include "RekordboxLookAndFeel.h"

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

    // Caches de miniaturas de waveform — UM PARA CADA RESOLUCAO.
    //
    // IMPORTE: o juce::AudioThumbnailCache indexa os thumbs pelo hashCode do
    // SOURCE (o arquivo), e NAO pela resolucao. Mini (1024 samples) e Big
    // (2048 samples) do MESMO arquivo cairiam no MESMO slot do cache, com
    // dados de resolucao errada — a waveform sairia corrompida ou vazia.
    // Por isso cada resolucao tem seu proprio cache.
    //
    // IMPORTANTE: declarados ANTES dos decks de proposito. Os membros sao
    // destruidos na ordem inversa da declaracao, e o destructor do
    // AudioThumbnail (dentro do MiniWaveformComponent de cada deck) acessa
    // este cache. Se o cache fosse destruido primeiro, teriamos um
    // use-after-free no encerramento do app.
    juce::AudioThumbnailCache miniThumbnailCache{ 5 };
    juce::AudioThumbnailCache bigThumbnailCache{ 5 };

    MixerDeviceScanner deviceScanner;
    juce::MixerAudioSource mixerSource;

    // Decks
    std::unique_ptr<DeckPlayer> deckPlayer1;
    std::unique_ptr<DeckPlayer> deckPlayer2;
    std::unique_ptr<DeckComponent> deck1;
    std::unique_ptr<DeckComponent> deck2;

    // Seções do layout raiz (docs/DOCUMENTACAO_LAYOUT_UI.md §2.1)
    std::unique_ptr<GlobalHeaderComponent> globalHeader;
    std::unique_ptr<BigWaveformsComponent> bigWaveforms;
    std::unique_ptr<MixerComponent> mixer;
    std::unique_ptr<LibraryComponent> library;

    // Diálogo de configurações de áudio (criado sob demanda)
    std::unique_ptr<SettingsDialog> settingsDialog;

    // Botão de settings
    juce::TextButton settingsButton{ "Settings" };

    // Look and feel personalizado.
    //
    // IMPORTANTE: este member DEVE ser o ULTIMO da classe. Os members sao
    // destruidos na ordem inversa da declaracao, e os widgets filhos usam este
    // LookAndFeel enquanto pintam. Se ele fosse destruido antes deles, o app
    // quebraria ao fechar. Ver docs/DOCUMENTACAO_LAYOUT_UI.md §6.
    RekordboxLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
