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
    currentSampleRate = sampleRate;

    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // Prepara os filtros na taxa do dispositivo.
    filterCoefficientsValid = false;
    updateFilterCoefficientsIfNeeded();

    applyGain();
}

void DeckPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Os coeficientes dos filtros so podem ser trocados quando o audio nao
    // esta passando por eles. Fazemo-lo aqui, na fronteira do bloco, ANTES
    // de processar: trocar no meio do bloco produz um clique audivel.
    updateFilterCoefficientsIfNeeded();

    resampleSource.getNextAudioBlock(bufferToFill);

    if (bufferToFill.buffer == nullptr || bufferToFill.numSamples <= 0)
        return;

    auto& buffer = *bufferToFill.buffer;
    const auto numSamples = bufferToFill.numSamples;
    const auto startSample = bufferToFill.startSample;

    // EQ de 3 bandas + filtro, in-place, canal a canal.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch, startSample);

        lowFilter.processSamples(samples, numSamples);
        midFilter.processSamples(samples, numSamples);
        highFilter.processSamples(samples, numSamples);

        // O filtro e bipolar: em valores negativos roda passa-alta, em
        // positivos passa-baixa. Perto do zero os dois ficam em bypass.
        if (appliedFilterAmount > 0.001f)
            filterLowPass.processSamples(samples, numSamples);
        else if (appliedFilterAmount < -0.001f)
            filterHighPass.processSamples(samples, numSamples);
    }

    // Nivel RMS do bloco, por canal, publicado ja em dB. A conversao RMS->dB
    // e cara demais para repetir a 30 Hz na thread de GUI.
    for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
    {
        const auto magnitude = buffer.getMagnitude(ch, startSample, numSamples);
        levelDb[ch].set(juce::Decibels::gainToDecibels(magnitude));
    }
}

void DeckPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

//==============================================================================
void DeckPlayer::updateFilterCoefficientsIfNeeded()
{
    // Le os valores pedidos pela thread de GUI.
    const auto lowDb  = pendingEqLowDb.get();
    const auto midDb  = pendingEqMidDb.get();
    const auto highDb = pendingEqHighDb.get();
    const auto filter = pendingFilterAmount.get();

    const auto unchanged = filterCoefficientsValid
                           && lowDb  == appliedEqLowDb
                           && midDb  == appliedEqMidDb
                           && highDb == appliedEqHighDb
                           && filter == appliedFilterAmount;

    if (unchanged)
        return;

    const auto rate = currentSampleRate;

    // EQ de 3 bandas. As factories de shelf e peak recebem o ganho como
    // FATOR LINEAR, nao em dB.
    lowFilter.setCoefficients(juce::IIRCoefficients::makeLowShelf(
        rate, 250.0, 0.7f, juce::Decibels::decibelsToGain(lowDb)));

    midFilter.setCoefficients(juce::IIRCoefficients::makePeakFilter(
        rate, 1000.0, 1.0f, juce::Decibels::decibelsToGain(midDb)));

    highFilter.setCoefficients(juce::IIRCoefficients::makeHighShelf(
        rate, 4000.0, 0.7f, juce::Decibels::decibelsToGain(highDb)));

    // Filtro bipolar: passa-baixa quando o knob sobe, passa-alta quando desce.
    if (filter > 0.001f)
    {
        const auto cutoff = juce::jlimit(20.0f, 20000.0f,
                            juce::jmap(filter, 0.0f, 1.0f, 20000.0f, 20.0f));

        filterLowPass.setCoefficients(juce::IIRCoefficients::makeLowPass(rate, cutoff, 0.7f));
    }
    else if (filter < -0.001f)
    {
        const auto cutoff = juce::jlimit(20.0f, 20000.0f,
                            juce::jmap(filter, -1.0f, 0.0f, 20.0f, 20000.0f));

        filterHighPass.setCoefficients(juce::IIRCoefficients::makeHighPass(rate, cutoff, 0.7f));
    }

    appliedEqLowDb  = lowDb;
    appliedEqMidDb  = midDb;
    appliedEqHighDb = highDb;
    appliedFilterAmount = filter;
    filterCoefficientsValid = true;
}

//==============================================================================
void DeckPlayer::applyGain()
{
    // Ganho linear total = trim x fader x crossfader.
    const auto linear = juce::Decibels::decibelsToGain(trimDb)
                      * juce::jlimit(0.0f, 1.0f, static_cast<float>(channelGain))
                      * juce::jlimit(0.0f, 1.0f, static_cast<float>(crossfadeGain));

    transportSource.setGain(linear);
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
    channelGain = juce::jlimit(0.0, 1.0, gain);
    applyGain();
}

double DeckPlayer::getGain() const { return channelGain; }

//==============================================================================
void DeckPlayer::setTrimDb(double db)
{
    trimDb = juce::jlimit(-12.0, 12.0, db);
    applyGain();
}

//==============================================================================
void DeckPlayer::setEqLowDb(double db)
{
    eqLowDb = juce::jlimit(-12.0, 12.0, db);
    pendingEqLowDb.set(static_cast<float>(eqLowDb));
}

void DeckPlayer::setEqMidDb(double db)
{
    eqMidDb = juce::jlimit(-12.0, 12.0, db);
    pendingEqMidDb.set(static_cast<float>(eqMidDb));
}

void DeckPlayer::setEqHighDb(double db)
{
    eqHighDb = juce::jlimit(-12.0, 12.0, db);
    pendingEqHighDb.set(static_cast<float>(eqHighDb));
}

//==============================================================================
void DeckPlayer::setFilterAmount(double amount)
{
    filterAmount = juce::jlimit(-1.0, 1.0, amount);
    pendingFilterAmount.set(static_cast<float>(filterAmount));
}

//==============================================================================
void DeckPlayer::setCrossfadeGain(double gain)
{
    crossfadeGain = juce::jlimit(0.0, 1.0, gain);
    applyGain();
}

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

double DeckPlayer::getPositionInSeconds() const
{
    return transportSource.getCurrentPosition();
}

double DeckPlayer::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

//==============================================================================
float DeckPlayer::getLevelDb(int channel) const
{
    if (channel < 0 || channel > 1)
        return -100.0f;

    return levelDb[channel].get();
}

//==============================================================================
juce::Atomic<float>& DeckPlayer::getLevelSource (int channel)
{
    // O canal e fixado em 0: cada DeckPlayer tem o seu proprio par de atomics.
    return levelDb[juce::jlimit (0, 1, channel)];
}
