#include "BigWaveformsComponent.h"
#include "RekordboxColours.h"

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
    g.fillAll (RekordboxColours::background);
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
void BigWaveformsComponent::timerCallback()
{
    // As ondas seguem a reproducao dos respectivos decks.
    if (playerA != nullptr)
        waveformA.setPositionSeconds (playerA->getPositionInSeconds());

    if (playerB != nullptr)
        waveformB.setPositionSeconds (playerB->getPositionInSeconds());
}

//==============================================================================
void BigWaveformsComponent::loadURL (const juce::URL& url)
{
    waveformA.loadURL (url);
    waveformB.loadURL (url);
}
