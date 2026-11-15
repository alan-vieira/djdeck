#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <thread>
#include <functional>

//==============================================================================
// Detector de BPM por analise de áudio.
//
// Pipeline:
//   1. Decodifica os primeiros segundos do arquivo para mono.
//   2. Calcula um envelope de onsets usando fluxo espectral (FFT de 2048,
//      salto de 512, janela de Hann).
//   3. Autocorrela o envelope nos lags correspondentes a 60..200 BPM e
//      escolhe o pico com maior correlacao.
//   4. Retorna 0.0 quando nao ha sinal suficiente ou o resultado nao tem
//      confianca.
//
// Sem dependencias alem do proprio JUCE.
class BpmDetector
{
public:
    // Estima o BPM a partir de um reader ja aberto. Retorna 0.0 em caso de
    // falha.
    static double estimate (juce::AudioFormatReader& reader, const std::atomic<bool>* shouldCancel = nullptr);

    // Dispara a estimativa numa thread de fundo e avisa via callback,
    // executado na message thread (seguro para atualizar a UI).
    static void detectAsync (const juce::File& file,
                             juce::AudioFormatManager& formatManager,
                             std::function<void (double bpm)> callback);

};
