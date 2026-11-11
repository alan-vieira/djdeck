#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "BigWaveformComponent.h"
#include "DeckPlayer.h"

//==============================================================================
// As duas ondas rolantes do topo: deck 1 acima, deck 2 abaixo.
//
// Cada BigWaveform tem a agulha FIXA no centro e a onda rola por baixo dela.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §4.1.
class BigWaveformsComponent : public juce::Component,
                              public juce::Timer
{
public:
    BigWaveformsComponent (juce::AudioFormatManager& formatManagerToUse,
                           juce::AudioThumbnailCache& cacheToUse,
                           DeckPlayer* playerAToUse,
                           DeckPlayer* playerBToUse);
    ~BigWaveformsComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Timer: mantem as duas ondas sincronizadas com a reproducao.
    void timerCallback() override;

    // Carrega a mesma faixa nos dois componentes (usado no carregamento).
    void loadURL (const juce::URL& url);

    // Acesso as ondas, para o cabecalho do deck marcar os hot cues.
    BigWaveformComponent& getWaveformA() { return waveformA; }
    BigWaveformComponent& getWaveformB() { return waveformB; }

private:
    BigWaveformComponent waveformA;
    BigWaveformComponent waveformB;

    DeckPlayer* playerA;
    DeckPlayer* playerB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BigWaveformsComponent)
};
