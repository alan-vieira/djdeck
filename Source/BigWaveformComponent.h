#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// Onda rolante de topo, com agulha FIXA no centro.
//
// O comportamento e oposto ao da waveform do deck:
//   - BigWaveform: agulha fixa no centro; a onda rola por baixo dela.
//   - MiniWaveform: agulha movel; a faixa inteira fica visivel e parada.
//
// A janela de tempo visivel e simetrica em torno da posicao atual:
//
//     visivel = [ posicao - janela/2 , posicao + janela/2 ]
//
// Como a janela e simetrica, `posicao` cai exatamente no pixel central — e a
// agulha e desenhada em width/2, sem depender do calculo da janela.
//
// No instante posicao = 0, a metade esquerda cai antes do inicio da faixa e
// fica vazia: e exatamente o "inicio cravado na agulha" do Rekordbox.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §4.1.

// Um beat da grade. `isDownbeat` marca o primeiro beat de cada compasso.
struct BeatMarker
{
    double timeSeconds = 0.0;
    bool isDownbeat = false;
};

class BigWaveformComponent : public juce::Component,
                             public juce::ChangeListener
{
public:
    // O AudioThumbnail precisa do format manager (para decodificar o arquivo)
    // e do cache (para reaproveitar picos ja computados entre sessoes).
    BigWaveformComponent (juce::AudioFormatManager& formatManagerToUse,
                          juce::AudioThumbnailCache& cacheToUse);
    ~BigWaveformComponent() override;

    void paint (juce::Graphics& g) override;

    // Carrega uma URL de audio. Usa FileInputSource: o AudioThumbnail le o
    // arquivo em background thread em vez de manter o audio inteiro na memoria
    // (10 min a 44.1 kHz stereo = ~212 MB por faixa).
    void loadURL (const juce::URL& url);

    // Define a posicao de reproducao, em segundos. A agulha fica fixa no
    // centro; a onda se desloca sob ela.
    void setPositionSeconds (double seconds);
    double getPositionSeconds() const { return positionSeconds; }

    // Grade de beats, em tempo absoluto (segundos). Vazio = sem grade.
    void setBeatGrid (const juce::Array<BeatMarker>& beats);
    const juce::Array<BeatMarker>& getBeatGrid() const { return beatGrid; }

    // Marcadores de hot cue: 8 posicoes em segundos. -1 = nao definido.
    void setHotCueTimes (const juce::Array<double>& timesInSeconds);
    const juce::Array<double>& getHotCueTimes() const { return hotCueTimes; }

    // Duracao da janela visivel, em segundos (padrao 30 s).
    void setWindowSeconds (double seconds);
    double getWindowSeconds() const { return windowSeconds; }

    // ChangeListener — o thumbnail avisa quando os picos ficam disponiveis.
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    void drawWaveform (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawBeatGrid (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawNeedle (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawCueMarkers (juce::Graphics& g, const juce::Rectangle<int>& area);

    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache& thumbnailCache;
    juce::AudioThumbnail audioThumb;

    double positionSeconds = 0.0;
    double windowSeconds = 30.0;

    juce::Array<BeatMarker> beatGrid;
    juce::Array<double> hotCueTimes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BigWaveformComponent)
};
