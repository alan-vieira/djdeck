#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

// Desenha a forma de onda completa de uma faixa usando AudioThumbnail
// e mostra um playhead (linha de posição) que se move durante a reprodução.
class WaveformDisplay : public juce::Component,
                        public juce::ChangeListener
{
public:
    WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
                    juce::AudioThumbnailCache& cacheToUse);
    ~WaveformDisplay() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Chamado quando a miniatura muda (após carregar arquivo)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Carrega uma URL de áudio e gera a miniatura
    void loadURL(juce::URL audioURL);

    // Define a posição relativa do playhead (0.0 a 1.0)
    void setPositionRelative(double pos);

private:
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnail audioThumb;

    // Buffer que alimenta o thumbnail. Precisa ser member porque o
    // AudioThumbnail mantém o ponteiro e o usa numa thread de background.
    juce::AudioBuffer<float> waveformBuffer;

    bool fileLoaded = false;
    double position = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
