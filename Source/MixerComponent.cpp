#include "MixerComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
MixerComponent::MixerComponent (DeckPlayer* playerAToUse, DeckPlayer* playerBToUse)
    : channelA (playerAToUse, false, 0),   // deck 1: offset X = 0 dentro do mixer
      channelB (playerBToUse, true, 54),   // deck 2: offset X = 54 dentro do mixer
      playerA (playerAToUse),
      playerB (playerBToUse)
{
    addAndMakeVisible (channelA);
    addAndMakeVisible (channelB);
    addAndMakeVisible (crossfader);


    // ---- Knobs do deck 1 ----
    // Knobs do deck 2 ----
    // Botões CUE ----

    // O crossfader publica os ganhos; aplicamos nos DeckPlayers.
    crossfader.onGainsChanged = [this] (float gainA, float gainB)
    {
        if (playerA != nullptr)
            playerA->setCrossfadeGain (gainA);
        if (playerB != nullptr)
            playerB->setCrossfadeGain (gainB);
    };
}

MixerComponent::~MixerComponent() = default;

//==============================================================================
void MixerComponent::setCrossfaderPosition (double position)
{
    crossfader.setPosition (position);
}

//==============================================================================
void MixerComponent::sliderValueChanged (juce::Slider* slider)
{
    juce::ignoreUnused (slider);  // knobs e CUE agora sao gerenciados por ChannelStripComponent
}

//==============================================================================
void MixerComponent::paint (juce::Graphics& g)
{
    g.fillAll (djdeck::layout::colours::Mixerbackground);

    g.setColour (djdeck::layout::colours::Separator);
    g.drawRect (getLocalBounds(), 1);
}

//==============================================================================
void MixerComponent::resized()
{
    // Mixer: x672 y138 l108 a241 (medida do Figma: figmaNode8_1134)
    // Todas as coordenadas são absolutas do canvas do Figma, convertidas
    // para locais do mixer subtraindo (672, 138).
    const int mixerAbsX = 672;
    const int mixerAbsY = 137;

    auto xLocal = [&] (int figmaX)  { return figmaX - mixerAbsX; };
    auto yLocal = [&] (int figmaY)  { return figmaY - mixerAbsY; };
    auto xLocalf = [&] (double figmaX)  { return static_cast<int> (figmaX - mixerAbsX); };
    auto yLocalf = [&] (double figmaY)  { return static_cast<int> (figmaY - mixerAbsY); };

    // ---- Canais (54px cada) ----
    // Canais começam em (0, 0) dentro do mixer. Os elementos internos
    // (knobs, VU, CUE, faders) sao posicionados por ChannelStripComponent::resized()
    // usando coordenadas absolutas do mixer (figmaX - mixerAbsX - stripAbsX).
    channelA.setBounds (0, 0,
                        djdeck::layout::figmaNode8_1134::widthPx / 2,
                        djdeck::layout::figmaNode8_1134::heightPx);
    channelB.setBounds (djdeck::layout::figmaNode8_1134::widthPx / 2, 0,
                        djdeck::layout::figmaNode8_1134::widthPx / 2,
                        djdeck::layout::figmaNode8_1134::heightPx);

    // ---- Crossfader button ----
    // Figma: x719 y354 l12 a10  -> relativo ao mixer: x47 y216
    crossfader.setBounds (xLocal (djdeck::layout::figmaNodemixerCrossfaderBtn1::xPx),
                          yLocal (djdeck::layout::figmaNodemixerCrossfaderBtn1::yPx),
                          djdeck::layout::figmaNodemixerCrossfaderBtn1::widthPx,
                          djdeck::layout::figmaNodemixerCrossfaderBtn1::heightPx);


    // ---- Crossfader button (nó 8:2024 / 8:1151) ----
}
