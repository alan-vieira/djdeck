#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>

//==============================================================================
// Processador de áudio de um único deck.
//
// Encapsula AudioFormatManager -> AudioFormatReaderSource -> AudioTransportSource
// e expõe controles de transporte, ganho, velocidade e posição.
//
// CADEIA DE ÁUDIO (docs/DOCUMENTACAO_LAYOUT_UI.md §5.1):
//
//   AudioFormatReaderSource
//     -> AudioTransportSource        (transporte + pitch)
//       -> ResamplingAudioSource     (resample ratio)
//         -> [ TRIM + fader ]        <- ganho linear
//           -> [ EQ 3 bandas ]       <- low/mid/high
//             -> [ Filtro ]          <- passa-baixa/alta
//               -> [ crossfader ]    <- ganho do crossfader
//                 -> saida para o MixerAudioSource
//
// REGRA: o DeckPlayer e a UNICA classe que toca no audio. Os componentes de UI
// so chamam setters — nunca escrevem em coeficientes de filtro em uso.
//
// O crossfader NAO pode ser implementado no MixerAudioSource: ele nao tem
// controle de ganho por fonte. Por isso ele vive aqui, antes da soma.
class DeckPlayer : public juce::AudioSource
{
public:
    DeckPlayer(juce::AudioFormatManager& formatManagerToUse);
    ~DeckPlayer() override;

    // juce::AudioSource
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Transporte
    void loadURL(juce::URL audioURL);
    void start();
    void stop();
    bool isPlaying() const;

    // Ganho (0.0 a 1.0) — fader de canal (LEVEL).
    void setGain(double gain);
    double getGain() const;

    // Ganho de canal com nome explícito (usado pelo fader MIXING como balanço).
    void setChannelGain(double gain);

    // Estado CUE/PFL do deck.
    void setCue(bool on);
    bool isCue() const { return cueActive; }

    // TRIM — ganho de entrada, ±12 dB, neutro = 0 dB.
    void setTrimDb(double db);
    double getTrimDb() const { return trimDb; }

    // EQ de 3 bandas — ±12 dB cada, 0 dB = flat.
    void setEqLowDb(double db);
    void setEqMidDb(double db);
    void setEqHighDb(double db);
    double getEqLowDb() const  { return eqLowDb; }
    double getEqMidDb() const  { return eqMidDb; }
    double getEqHighDb() const { return eqHighDb; }

    // Filtro — -1.0 = passa-alta (20 kHz), 0.0 = neutro, +1.0 = passa-baixa (20 Hz).
    void setFilterAmount(double amount);
    double getFilterAmount() const { return filterAmount; }

    // Ganho do crossfader (0.0 a 1.0), multiplicado no ganho do canal.
    void setCrossfadeGain(double gain);
    double getCrossfadeGain() const { return crossfadeGain; }

    // Velocidade de reprodução (pitch/resampling)
    void setSpeed(double ratio);
    double getSpeed() const;

    // Posição
    void setPosition(double posInSeconds);
    void setPositionRelative(double pos); // 0.0 a 1.0
    double getPositionRelative() const;
    double getPositionInSeconds() const;
    double getLengthInSeconds() const;

    // Nivel por canal, em dB. Publicado pela thread de audio e lido pela thread
    // de GUI (thread-safe via juce::Atomic).
    float getLevelDb(int channel) const;

    // Referencia ao atomic que publica o nivel. O VUMeterComponent recebe
    // esta referencia e le o valor diretamente, sem copiar.
    //
    // O DeckPlayer e destruido DEPOIS dos componentes de UI, entao a
    // referencia permanece valida durante toda a vida da UI.
    juce::Atomic<float>& getLevelSource (int channel);

    // BPM da faixa. 0 quando desconhecido — a deteccao de BPM ainda nao
    // existe no djdeck (ver docs/ROADMAP.md).
    double getBpm() const { return bpm; }
    void setBpm(double value) { bpm = value; }

private:
    // Aplica o ganho total (trim x fader x crossfader) no transporte.
    void applyGain();

    // Troca os coeficientes do EQ/filtro de forma segura: os parametros
    // desejados sao publicados pela thread de GUI em atomicos, e os
    // coeficientes so sao trocados na fronteira de bloco, na thread de audio.
    // Trocar no meio do bloco produz um clique.
    void updateFilterCoefficientsIfNeeded();

    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

    // ---- Estado de DSP (thread de GUI escreve, thread de audio le) ----
    double trimDb = 0.0;
    bool cueActive = false;
    double eqLowDb = 0.0;
    double eqMidDb = 0.0;
    double eqHighDb = 0.0;
    double filterAmount = 0.0;
    double channelGain = 0.5;
    double crossfadeGain = 1.0;

    // Valores pedidos pela GUI, consumidos pela thread de audio.
    juce::Atomic<float> pendingEqLowDb{ 0.0f };
    juce::Atomic<float> pendingEqMidDb{ 0.0f };
    juce::Atomic<float> pendingEqHighDb{ 0.0f };
    juce::Atomic<float> pendingFilterAmount{ 0.0f };

    // Ultimos coeficientes efetivamente aplicados (thread de audio).
    float appliedEqLowDb = 0.0f;
    float appliedEqMidDb = 0.0f;
    float appliedEqHighDb = 0.0f;
    float appliedFilterAmount = 0.0f;
    bool filterCoefficientsValid = false;

    double currentSampleRate = 44100.0;
    double bpm = 0.0;

    // Filtros do EQ de 3 bandas: low shelf 250 Hz, peak 1 kHz, high shelf 4 kHz.
    juce::IIRFilter lowFilter;
    juce::IIRFilter midFilter;
    juce::IIRFilter highFilter;

    // Filtro passa-baixa/alta (bypass no centro).
    juce::IIRFilter filterLowPass;
    juce::IIRFilter filterHighPass;

    // Nivel RMS por canal, publicado ja em dB (a conversao e cara demais para
    // repetir a 30 Hz na thread de GUI).
    juce::Atomic<float> levelDb[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckPlayer)
};
