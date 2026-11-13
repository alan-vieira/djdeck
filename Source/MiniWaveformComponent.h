#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "BigWaveformComponent.h"

//==============================================================================
// Waveform do deck: agulha MOVEL, da esquerda para a direita.
//
// Este e o comportamento OPOSTO ao da BigWaveform:
//
//   - BigWaveform: agulha fixa no centro; a onda rola.
//   - MiniWaveform: agulha movel; a faixa inteira fica visivel e parada.
//
// A faixa completa e estatica (drawChannel de 0 ao total) e a agulha
// percorre acompanhando a reproducao.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §4.2.
class MiniWaveformComponent : public juce::Component,
                              public juce::ChangeListener
{
public:
    MiniWaveformComponent (juce::AudioFormatManager& formatManagerToUse,
                           juce::AudioThumbnailCache& cacheToUse);
    ~MiniWaveformComponent() override;

    void paint (juce::Graphics& g) override;

    // Chamado quando a miniatura muda (apos carregar arquivo)
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    // Carrega uma URL de audio e gera a miniatura.
    //
    // Usa FileInputSource: o AudioThumbnail le o arquivo em background thread
    // em vez de manter o audio inteiro na memoria (~212 MB por faixa de 10 min).
    void loadURL (juce::URL audioURL);

    // Define a posicao relativa do playhead (0.0 a 1.0)
    void setPositionRelative (double pos);

    // Diagnostico: o que o thumbnail ja sabe. total=0 significa que nenhuma
    // amostra foi processada ainda -> nada pode ser desenhado.
    double getThumbnailTotalLength() const noexcept { return audioThumb.getTotalLength(); }
    int getThumbnailChannels() const noexcept { return audioThumb.getNumChannels(); }

    // Grade de beats, em tempo absoluto (segundos). Vazio = sem grade.
    void setBeatGrid (const juce::Array<BeatMarker>& beats);

    // Marcadores de hot cue: 8 posicoes em segundos. -1 = nao definido.
    void setHotCueTimes (const juce::Array<double>& timesInSeconds);

    // Altura da faixa de overview (miniatura) acima da waveform, em pixels.
    // 0 = sem overview.
    void setOverviewHeight (int pixels);
    int getOverviewHeight() const { return overviewHeight; }

private:
    void drawOverview (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawWaveform (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawBeatGrid (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawCueMarkers (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawPlayhead (juce::Graphics& g, const juce::Rectangle<int>& area);

    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnail audioThumb;

    bool fileLoaded = false;
    double position = 0.0;

    juce::Array<BeatMarker> beatGrid;
    juce::Array<double> hotCueTimes;

    int overviewHeight = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiniWaveformComponent)
};
