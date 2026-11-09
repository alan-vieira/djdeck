#include "WaveformDisplay.h"

//==============================================================================
WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
                                 juce::AudioThumbnailCache& cacheToUse)
    : formatManager(formatManagerToUse),
      audioThumb(1024, formatManagerToUse, cacheToUse)
{
    audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
    audioThumb.removeChangeListener(this);
}

//==============================================================================
void WaveformDisplay::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll(juce::Colours::black);

    if (!fileLoaded)
    {
        g.setColour(juce::Colours::grey);
        g.setFont(14.0f);
        g.drawText("Nenhuma faixa carregada", area,
                   juce::Justification::centred, true);
        return;
    }

    // Desenha a forma de onda em laranja
    g.setColour(juce::Colours::orange);
    audioThumb.drawChannel(g, area, 0.0, audioThumb.getTotalLength(), 0, 1.0f);

    // Desenha o playhead (linha vertical na posição atual)
    if (audioThumb.getTotalLength() > 0)
    {
        auto playheadX = static_cast<float>(position) * static_cast<float>(area.getWidth());
        g.setColour(juce::Colours::white);
        g.drawLine(playheadX, static_cast<float>(area.getY()),
                   playheadX, static_cast<float>(area.getBottom()), 2.0f);
    }
}

void WaveformDisplay::resized()
{
    // Layout simples - o componente preenche a área disponível
}

//==============================================================================
void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioThumb)
    {
        fileLoaded = true;
        repaint();
    }
}

void WaveformDisplay::loadURL(juce::URL audioURL)
{
    fileLoaded = false;
    position = 0.0;

    juce::File file = audioURL.getLocalFile();
    if (!file.exists())
        return;

    std::unique_ptr<juce::AudioFormatReader> reader;
    reader.reset(formatManager.createReaderFor(file));
    if (reader == nullptr)
        return;

    // O AudioThumbnail NÃO copia os dados do buffer: ele mantém o ponteiro e o
    // lê numa thread de background. Por isso o buffer precisa viver enquanto o
    // thumbnail o usa -> guardamos como member, não como variável local.
    waveformBuffer.setSize((int)reader->numChannels,
                           (int)reader->lengthInSamples,
                           false, true, false);

    reader->read(&waveformBuffer, 0, waveformBuffer.getNumSamples(), 0, true, true);

    // hashCode único por arquivo: evita que o thumbnail cache misture arquivos
    // diferentes que compartilhem o mesmo código.
    audioThumb.setSource(&waveformBuffer, reader->sampleRate, file.hashCode64());

    fileLoaded = true;
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    position = pos;
    repaint();
}
