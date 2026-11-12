#include "MiniWaveformComponent.h"
#include "RekordboxColours.h"

//==============================================================================
MiniWaveformComponent::MiniWaveformComponent (juce::AudioFormatManager& formatManagerToUse,
                                              juce::AudioThumbnailCache& cacheToUse)
    : formatManager (formatManagerToUse),
      audioThumb (1024, formatManagerToUse, cacheToUse)
{
    audioThumb.addChangeListener (this);
}

MiniWaveformComponent::~MiniWaveformComponent()
{
    audioThumb.removeChangeListener (this);
}

//==============================================================================
void MiniWaveformComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (RekordboxColours::canvas);

    if (! fileLoaded)
    {
        g.setColour (RekordboxColours::textTertiary);
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawText ("Nenhuma faixa carregada", area, juce::Justification::centred, true);
        return;
    }

    // ---- Overview: miniatura fina acima, com a posicao atual ----
    if (overviewHeight > 0)
    {
        auto overviewArea = area.removeFromTop (overviewHeight);
        drawOverview (g, overviewArea);
    }

    // ---- Onda ----
    drawWaveform (g, area);
    drawBeatGrid (g, area);
    drawCueMarkers (g, area);
    drawPlayhead (g, area);
}

//==============================================================================
void MiniWaveformComponent::drawOverview (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour (RekordboxColours::panel);
    g.fillRect (area);

    const auto totalLength = audioThumb.getTotalLength();

    if (totalLength <= 0.0)
        return;

    // A faixa inteira, desenhada na altura da miniatura.
    g.setColour (RekordboxColours::textDisabled);
    audioThumb.drawChannel (g, area, 0.0, totalLength, 0, 1.0f);

    // A posicao atual, destacada.
    const auto x = area.getX() + static_cast<int> (position * area.getWidth());

    g.setColour (RekordboxColours::accent);
    g.fillRect (x, area.getY(), 2, area.getHeight());
}

//==============================================================================
void MiniWaveformComponent::drawWaveform (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    const auto totalLength = audioThumb.getTotalLength();

    if (totalLength <= 0.0)
        return;

    // Gradiente vertical: ciano no topo, azul na base.
    juce::ColourGradient waveGradient (RekordboxColours::waveMid, 0.0f, 0.0f,
                                       RekordboxColours::waveLow, 0.0f, 1.0f, false);
    waveGradient.addColour (0.5f, RekordboxColours::waveHigh);

    // A faixa completa fica visivel e parada: drawChannel de 0 ao total.
    g.setGradientFill (waveGradient);
    audioThumb.drawChannel (g, area, 0.0, totalLength, 0, 1.0f);
    g.setColour (juce::Colours::transparentBlack);
}

//==============================================================================
void MiniWaveformComponent::drawBeatGrid (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (beatGrid.isEmpty())
        return;

    const auto totalLength = audioThumb.getTotalLength();

    if (totalLength <= 0.0)
        return;

    const auto pxPerSecond = static_cast<float> (area.getWidth()) / static_cast<float> (totalLength);
    const auto top = static_cast<float> (area.getY());
    const auto bottom = static_cast<float> (area.getBottom());

    // Batidas finas
    g.setColour (RekordboxColours::beatGrid.withAlpha (0.35f));

    for (const auto& beat : beatGrid)
    {
        if (beat.isDownbeat)
            continue;

        const auto x = static_cast<float> (area.getX()) + static_cast<float> (beat.timeSeconds * pxPerSecond);

        if (x >= static_cast<float> (area.getX()) && x < static_cast<float> (area.getRight()))
            g.drawLine (x, top, x, bottom, 1.0f);
    }

    // Downbeats
    g.setColour (RekordboxColours::downbeat);

    for (const auto& beat : beatGrid)
    {
        if (! beat.isDownbeat)
            continue;

        const auto x = static_cast<float> (area.getX()) + static_cast<float> (beat.timeSeconds * pxPerSecond);

        if (x >= static_cast<float> (area.getX()) && x < static_cast<float> (area.getRight()))
            g.drawLine (x, top, x, bottom, 2.0f);
    }
}

//==============================================================================
void MiniWaveformComponent::drawCueMarkers (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (hotCueTimes.isEmpty())
        return;

    const auto totalLength = audioThumb.getTotalLength();

    if (totalLength <= 0.0)
        return;

    const auto pxPerSecond = static_cast<float> (area.getWidth()) / static_cast<float> (totalLength);

    for (int i = 0; i < hotCueTimes.size() && i < 8; ++i)
    {
        const auto time = hotCueTimes[i];

        if (time < 0.0)
            continue;

        const auto x = static_cast<float> (area.getX()) + static_cast<float> (time * pxPerSecond);

        if (x < static_cast<float> (area.getX()) || x >= static_cast<float> (area.getRight()))
            continue;

        // Marcador na borda superior, na cor do pad correspondente.
        g.setColour (RekordboxColours::hotCue[i]);
        g.fillRect (x - 1.0f, static_cast<float> (area.getY()), 2.0f,
                    static_cast<float> (area.getHeight() / 4));
    }
}

//==============================================================================
void MiniWaveformComponent::drawPlayhead (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    const auto totalLength = audioThumb.getTotalLength();

    if (totalLength <= 0.0)
        return;

    // A agulha percorre da esquerda para a direita acompanhando a reproducao.
    const auto playheadX = static_cast<float> (area.getX())
                         + static_cast<float> (position * area.getWidth());

    g.setColour (RekordboxColours::needle);
    g.drawLine (playheadX, static_cast<float> (area.getY()),
                playheadX, static_cast<float> (area.getBottom()), 2.0f);

    // Marcadores triangulares no topo e na base.
    juce::Path topMarker;
    topMarker.addTriangle (playheadX - 4.0f, static_cast<float> (area.getY()),
                           playheadX + 4.0f, static_cast<float> (area.getY()),
                           playheadX,         static_cast<float> (area.getY()) + 6.0f);

    juce::Path bottomMarker;
    bottomMarker.addTriangle (playheadX - 4.0f, static_cast<float> (area.getBottom()),
                              playheadX + 4.0f, static_cast<float> (area.getBottom()),
                              playheadX,         static_cast<float> (area.getBottom()) - 6.0f);

    g.fillPath (topMarker);
    g.fillPath (bottomMarker);
}

//==============================================================================
void MiniWaveformComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &audioThumb)
    {
        fileLoaded = true;
        repaint();
    }
}

//==============================================================================
void MiniWaveformComponent::loadURL (juce::URL audioURL)
{
    fileLoaded = false;
    position = 0.0;

    const auto file = audioURL.getLocalFile();

    if (! file.existsAsFile())
    {
        audioThumb.setSource (nullptr);
        repaint();
        return;
    }

    // O thumbnail assume a posse do source e o destroi quando substituido.
    // FileInputSource abre o arquivo so quando precisa — nao carrega o audio
    // na memoria.
    audioThumb.setSource (nullptr);
    fileLoaded = audioThumb.setSource (new juce::FileInputSource (file));

    repaint();
}

//==============================================================================
void MiniWaveformComponent::setPositionRelative (double pos)
{
    position = juce::jlimit (0.0, 1.0, pos);
    repaint();
}

//==============================================================================
void MiniWaveformComponent::setBeatGrid (const juce::Array<BeatMarker>& beats)
{
    beatGrid = beats;
    repaint();
}

//==============================================================================
void MiniWaveformComponent::setHotCueTimes (const juce::Array<double>& timesInSeconds)
{
    hotCueTimes = timesInSeconds;
    repaint();
}

//==============================================================================
void MiniWaveformComponent::setOverviewHeight (int pixels)
{
    overviewHeight = juce::jmax (0, pixels);
    repaint();
}
