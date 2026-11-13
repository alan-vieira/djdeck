#include "BigWaveformComponent.h"
#include "RekordboxColours.h"

//==============================================================================
BigWaveformComponent::BigWaveformComponent (juce::AudioFormatManager& formatManagerToUse,
                                            juce::AudioThumbnailCache& cacheToUse)
    : formatManager (formatManagerToUse),
      thumbnailCache (cacheToUse),
      audioThumb (2048, formatManagerToUse, cacheToUse)
{
    audioThumb.addChangeListener (this);
}

BigWaveformComponent::~BigWaveformComponent()
{
    audioThumb.removeChangeListener (this);
}

//==============================================================================
void BigWaveformComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (RekordboxColours::canvas);

    if (! fileLoaded || audioThumb.getTotalLength() <= 0.0)
    {
        g.setColour (RekordboxColours::textTertiary);
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawText ("Nenhuma faixa carregada", area,
                    juce::Justification::centred, true);
        return;
    }

    drawWaveform (g, area);
    drawBeatGrid (g, area);
    drawCueMarkers (g, area);
    drawNeedle (g, area);
}

//==============================================================================
void BigWaveformComponent::drawWaveform (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    // Janela simetrica em torno da posicao: o ponto central do retangulo
    // corresponde exatamente ao instante `positionSeconds`.
    //
    // No instante posicao = 0 a janela e [-janela/2, +janela/2]: a metade
    // esquerda cai antes do inicio da faixa e fica vazia — e exatamente o
    // "inicio cravado na agulha" do Rekordbox.
    const auto startTime = positionSeconds - windowSeconds * 0.5;
    const auto endTime   = positionSeconds + windowSeconds * 0.5;

    // A onda e desenhada com gradiente vertical: ciano no topo, azul na base.
    // E o que distingue a onda em camadas da onda simples.
    juce::ColourGradient waveGradient (RekordboxColours::waveMid, 0.0f, 0.0f,
                                       RekordboxColours::waveLow, 0.0f, 1.0f, false);
    waveGradient.addColour (0.5f, RekordboxColours::waveHigh);

    g.setGradientFill (waveGradient);
    audioThumb.drawChannel (g, area, startTime, endTime, 0, 1.0f);

    // Limpa o gradiente: as chamadas seguintes (grade, agulha) usam cores
    // solidas e um gradiente residual as contaminaria.
    g.setColour (juce::Colours::transparentBlack);
}

//==============================================================================
void BigWaveformComponent::drawBeatGrid (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (beatGrid.isEmpty() || windowSeconds <= 0.0)
        return;

    const auto pxPerSecond = static_cast<float> (area.getWidth()) / static_cast<float> (windowSeconds);
    const auto startTime = positionSeconds - windowSeconds * 0.5;

    // drawLine() recebe float no JUCE 9 — oRectangle<int> devolve int.
    const auto top = static_cast<float> (area.getY());
    const auto bottom = static_cast<float> (area.getBottom());

    // Batidas finas
    g.setColour (RekordboxColours::beatGrid.withAlpha (0.35f));

    for (const auto& beat : beatGrid)
    {
        if (beat.isDownbeat)
            continue;

        const auto x = area.getX() + static_cast<int> ((beat.timeSeconds - startTime) * pxPerSecond);

        if (x >= area.getX() && x < area.getRight())
            g.drawLine (static_cast<float> (x), top, static_cast<float> (x), bottom, 1.0f);
    }

    // Downbeats — mais grossos
    g.setColour (RekordboxColours::downbeat);

    for (const auto& beat : beatGrid)
    {
        if (! beat.isDownbeat)
            continue;

        const auto x = area.getX() + static_cast<int> ((beat.timeSeconds - startTime) * pxPerSecond);

        if (x >= area.getX() && x < area.getRight())
            g.drawLine (static_cast<float> (x), top, static_cast<float> (x), bottom, 2.0f);
    }
}

//==============================================================================
void BigWaveformComponent::drawCueMarkers (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (hotCueTimes.isEmpty() || windowSeconds <= 0.0)
        return;

    const auto pxPerSecond = static_cast<float> (area.getWidth()) / static_cast<float> (windowSeconds);
    const auto startTime = positionSeconds - windowSeconds * 0.5;

    for (int i = 0; i < hotCueTimes.size() && i < 8; ++i)
    {
        const auto time = hotCueTimes[i];

        if (time < 0.0)
            continue;

        const auto x = area.getX() + static_cast<int> ((time - startTime) * pxPerSecond);

        if (x < area.getX() || x >= area.getRight())
            continue;

        // Triangulo na borda superior, na cor do pad correspondente.
        juce::Path marker;
        marker.addTriangle (static_cast<float> (x) - 4.0f, static_cast<float> (area.getY()),
                            static_cast<float> (x) + 4.0f, static_cast<float> (area.getY()),
                            static_cast<float> (x),         static_cast<float> (area.getY()) + 6.0f);

        g.setColour (RekordboxColours::hotCue[i]);
        g.fillPath (marker);
    }
}

//==============================================================================
void BigWaveformComponent::drawNeedle (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    // A agulha NUNCA se move: ela fica no centro, e a onda rola por baixo.
    const auto needleX = static_cast<float> (area.getCentreX());

    g.setColour (RekordboxColours::needle);
    g.drawLine (needleX, static_cast<float> (area.getY()),
                needleX, static_cast<float> (area.getBottom()), 1.0f);

    // Marcadores triangulares no topo e na base, como no Rekordbox.
    juce::Path topMarker;
    topMarker.addTriangle (static_cast<float> (needleX) - 4.0f, static_cast<float> (area.getY()),
                           static_cast<float> (needleX) + 4.0f, static_cast<float> (area.getY()),
                           static_cast<float> (needleX),         static_cast<float> (area.getY()) + 6.0f);

    juce::Path bottomMarker;
    bottomMarker.addTriangle (static_cast<float> (needleX) - 4.0f, static_cast<float> (area.getBottom()),
                              static_cast<float> (needleX) + 4.0f, static_cast<float> (area.getBottom()),
                              static_cast<float> (needleX),         static_cast<float> (area.getBottom()) - 6.0f);

    g.setColour (RekordboxColours::needle);
    g.fillPath (topMarker);
    g.fillPath (bottomMarker);
}

//==============================================================================
void BigWaveformComponent::loadURL (const juce::URL& url)
{
    const auto file = url.getLocalFile();

    positionSeconds = 0.0;

    if (! file.existsAsFile())
    {
        audioThumb.setSource (nullptr);
        repaint();
        return;
    }

    // O thumbnail assume a posse do source e o destroi quando substituido.
    //
    // FileInputSource abre o arquivo SO quando precisa, numa background thread.
    // A alternativa (decodificar o arquivo inteiro para um AudioBuffer em
    // memoria) travava a message thread durante a carga: 4:18 a 44.1 kHz
    // stereo sao ~91 MB para alocar e decodificar, o que congelava a UI
    // por centenas de milissegundos a cada faixa carregada.
    audioThumb.setSource (nullptr);
    fileLoaded = audioThumb.setSource (new juce::FileInputSource (file));

    repaint();
}

//==============================================================================
void BigWaveformComponent::setPositionSeconds (double seconds)
{
    positionSeconds = juce::jmax (0.0, seconds);
    repaint();
}

//==============================================================================
void BigWaveformComponent::setBeatGrid (const juce::Array<BeatMarker>& beats)
{
    beatGrid = beats;
    repaint();
}

//==============================================================================
void BigWaveformComponent::setHotCueTimes (const juce::Array<double>& timesInSeconds)
{
    hotCueTimes = timesInSeconds;
    repaint();
}

//==============================================================================
void BigWaveformComponent::setWindowSeconds (double seconds)
{
    windowSeconds = juce::jmax (1.0, seconds);
    repaint();
}

//==============================================================================
void BigWaveformComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &audioThumb)
        repaint();
}
