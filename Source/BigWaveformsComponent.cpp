#include "BigWaveformsComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
BigWaveformsComponent::BigWaveformsComponent (juce::AudioFormatManager& formatManagerToUse,
                                              juce::AudioThumbnailCache& cacheToUse,
                                              DeckPlayer* playerAToUse,
                                              DeckPlayer* playerBToUse)
    : waveformA (formatManagerToUse, cacheToUse),
      waveformB (formatManagerToUse, cacheToUse),
      playerA (playerAToUse),
      playerB (playerBToUse)
{
    addAndMakeVisible (waveformA);
    addAndMakeVisible (waveformB);

    // ~30 fps, igual ao timer dos decks.
    startTimer (33);
}

BigWaveformsComponent::~BigWaveformsComponent()
{
    stopTimer();
}

//==============================================================================
void BigWaveformsComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (djdeck::layout::colours::Background);

    const auto halfHeight = djdeck::layout::figmaNodebigwaveform1::heightPx;

    // ---- Labels de track (conforme o HTML de referencia) ----
    // Deck 1 (azul) — banda superior (y=0..71)
    g.setColour (djdeck::layout::colours::Accent);
    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 11.0f, juce::Font::bold)));
    g.drawText ("DECK 1 — " + trackTitleA,
                juce::Rectangle<int> (12, 8, area.getWidth() - 24, 16),
                juce::Justification::centredLeft, false);

    // Deck 2 (vermelho) — banda inferior (y=71..137); o label e 8 px abaixo
    // do topo da SUA banda, como no HTML.
    g.setColour (djdeck::layout::colours::Cue);
    g.drawText ("DECK 2 — " + trackTitleB,
                juce::Rectangle<int> (12, halfHeight + 8, area.getWidth() - 24, 16),
                juce::Justification::centredLeft, false);
}

//==============================================================================
void BigWaveformsComponent::resized()
{
    // Alturas EXATAS dos nos do Figma (bigwaveform1: 71 px; bigwaveform2:
    // 66 px). A divisao em partes iguais (68/69) e o `reduced (2)` deixavam
    // as ondas fora das medidas do Figma e do prototipo HTML.
    const auto heightA = djdeck::layout::figmaNodebigwaveform1::heightPx;
    const auto heightB = djdeck::layout::figmaNodebigwaveform2::heightPx;

    waveformA.setBounds (0, 0, getWidth(), heightA);
    waveformB.setBounds (0, heightA, getWidth(), heightB);
}

//==============================================================================
void BigWaveformsComponent::setTrackTitle (int deckId, const juce::String& title)
{
    if (deckId == 1)
        trackTitleA = title;
    else if (deckId == 2)
        trackTitleB = title;
    repaint();
}

//==============================================================================
void BigWaveformsComponent::timerCallback()
{
    // As ondas seguem a reproducao dos respectivos decks.
    if (playerA != nullptr)
        waveformA.setPositionSeconds (playerA->getPositionInSeconds());

    if (playerB != nullptr)
        waveformB.setPositionSeconds (playerB->getPositionInSeconds());
}

//==============================================================================
void BigWaveformsComponent::loadURL(int deckId, const juce::URL& url)
{
    if (deckId == 1)
        waveformA.loadURL(url);
    else if (deckId == 2)
        waveformB.loadURL(url);
}
