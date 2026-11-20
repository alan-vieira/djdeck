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
    // Mixer: x672 y138 l108 a241 (medida do Figma: figmaNodemixer)
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
                        djdeck::layout::figmaNodemixer::widthPx / 2,
                        djdeck::layout::figmaNodemixer::heightPx);
    channelB.setBounds (djdeck::layout::figmaNodemixer::widthPx / 2, 0,
                        djdeck::layout::figmaNodemixer::widthPx / 2,
                        djdeck::layout::figmaNodemixer::heightPx);

    // ---- Crossfader ----
    // O componente dono do crossfader cobre a regiao do Figma que contem o
    // trilho (crossfader rail1: 99.39 x 2 em 678,358) e o botao
    // (crossfader btn1: 11.93 x 10 em 721,354). Antes ele recebia apenas o
    // retangulo do BOTAO (11.93 x 10), o que recortava o trilho inteiro.
    //
    // Coordenadas do Figma -> locais do mixer (subtrai 672, 137).
    const auto crossfaderAbsX = juce::jmin (djdeck::layout::figmaNodemixerCrossfaderRail1::xPx,
                                            djdeck::layout::figmaNodemixerCrossfaderBtn1::xPx);
    const auto crossfaderAbsY = juce::jmin (djdeck::layout::figmaNodemixerCrossfaderRail1::yPx,
                                            djdeck::layout::figmaNodemixerCrossfaderBtn1::yPx);
    const auto crossfaderAbsRight = juce::jmax (
        djdeck::layout::figmaNodemixerCrossfaderRail1::xPx + djdeck::layout::figmaNodemixerCrossfaderRail1::widthPx,
        djdeck::layout::figmaNodemixerCrossfaderBtn1::xPx + djdeck::layout::figmaNodemixerCrossfaderBtn1::widthPx);
    const auto crossfaderAbsBottom = juce::jmax (
        djdeck::layout::figmaNodemixerCrossfaderRail1::yPx + djdeck::layout::figmaNodemixerCrossfaderRail1::heightPx,
        djdeck::layout::figmaNodemixerCrossfaderBtn1::yPx + djdeck::layout::figmaNodemixerCrossfaderBtn1::heightPx);

    crossfader.setBounds (xLocalf (crossfaderAbsX),
                          yLocal (crossfaderAbsY),
                          static_cast<int> (crossfaderAbsRight - crossfaderAbsX),
                          static_cast<int> (crossfaderAbsBottom - crossfaderAbsY));
}
