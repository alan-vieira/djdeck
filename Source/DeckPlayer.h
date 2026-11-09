#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>

// Processador de áudio de um único deck.
// Encapsula AudioFormatManager -> AudioFormatReaderSource -> AudioTransportSource
// e expõe controles de transporte, ganho, velocidade e posição.
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

    // Ganho (0.0 a 1.0)
    void setGain(double gain);
    double getGain() const;

    // Velocidade de reprodução (pitch/resampling)
    void setSpeed(double ratio);
    double getSpeed() const;

    // Posição
    void setPosition(double posInSeconds);
    void setPositionRelative(double pos); // 0.0 a 1.0
    double getPositionRelative() const;
    double getLengthInSeconds() const;

private:
    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckPlayer)
};
