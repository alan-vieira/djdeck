#include "DeckPlayer.h"

//==============================================================================
DeckPlayer::DeckPlayer(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse),
      resampleSource(&transportSource, false, 2)
{
}

DeckPlayer::~DeckPlayer()
{
    releaseResources();
}

//==============================================================================
void DeckPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DeckPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);
}

void DeckPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

//==============================================================================
void DeckPlayer::loadURL(juce::URL audioURL)
{
    juce::File file = audioURL.getLocalFile();
    if (!file.existsAsFile())
        return;

    // createReaderFor() retorna um ponteiro cru: temos que devolvê-lo ao
    // AudioFormatReaderSource (que o deleta quando destruído).
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
    if (reader == nullptr)
        return;

    // Guarda a taxa ANTES de soltar o reader: o transporte precisa corrigir a
    // taxa do arquivo para a taxa do dispositivo de saída.
    const double sourceSampleRate = reader->sampleRate;

    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);

    transportSource.setSource(newSource.get(), 0, nullptr, sourceSampleRate);

    readerSource = std::move(newSource);
}

void DeckPlayer::start()      { transportSource.start(); }
void DeckPlayer::stop()       { transportSource.stop(); }
bool  DeckPlayer::isPlaying() const { return transportSource.isPlaying(); }

void DeckPlayer::setGain(double gain)
{
    transportSource.setGain(static_cast<float>(juce::jlimit(0.0, 1.0, gain)));
}
double DeckPlayer::getGain() const { return transportSource.getGain(); }

void DeckPlayer::setSpeed(double ratio)
{
    resampleSource.setResamplingRatio(juce::jlimit(0.5, 3.0, ratio));
}
double DeckPlayer::getSpeed() const { return resampleSource.getResamplingRatio(); }

void DeckPlayer::setPosition(double posInSeconds)
{
    transportSource.setPosition(posInSeconds);
}

void DeckPlayer::setPositionRelative(double pos)
{
    pos = juce::jlimit(0.0, 1.0, pos);
    double posInSeconds = transportSource.getLengthInSeconds() * pos;
    setPosition(posInSeconds);
}

double DeckPlayer::getPositionRelative() const
{
    auto length = transportSource.getLengthInSeconds();
    if (length == 0) return 0.0;
    return transportSource.getCurrentPosition() / length;
}

double DeckPlayer::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}
