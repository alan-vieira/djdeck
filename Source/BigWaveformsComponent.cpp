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

    const auto halfHeight = area.getHeight() / 2;

    // ---- Labels de track (conforme o HTML de referência) ----
    // Deck 1 (azul) — top half
    g.setColour (djdeck::layout::colours::Accent);
    g.setFont (juce::Font (juce::FontOptions ("Segoe UI", 11.0f, juce::Font::bold)));
    g.drawText ("DECK 1 — " + trackTitleA,
                juce::Rectangle<int> (12, 8, area.getWidth() - 24, 16),
                juce::Justification::centredLeft, false);

    // Deck 2 (vermelho) — bottom half
    g.setColour (djdeck::layout::colours::Cue);
    g.drawText ("DECK 2 — " + trackTitleB,
                juce::Rectangle<int> (12, halfHeight + 8, area.getWidth() - 24, 16),
                juce::Justification::centredLeft, false);
}

//==============================================================================
void BigWaveformsComponent::resized()
{
    auto area = getLocalBounds();

    // Deck 1 em cima, deck 2 embaixo, com 2 px de folga entre as ondas.
    const auto halfHeight = area.getHeight() / 2;

    waveformA.setBounds (area.removeFromTop (halfHeight).reduced (2));
    waveformB.setBounds (area.reduced (2));
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
