#include "MixerComponent.h"
#include "RekordboxColours.h"

//==============================================================================
MixerComponent::MixerComponent (DeckPlayer* playerAToUse, DeckPlayer* playerBToUse)
    : channelA (playerAToUse, false),   // deck 1: nao espelhado
      channelB (playerBToUse, true),    // deck 2: espelhado
      playerA (playerAToUse),
      playerB (playerBToUse)
{
    addAndMakeVisible (channelA);
    addAndMakeVisible (channelB);
    addAndMakeVisible (crossfader);

    // ---- MIXING / LEVEL (rodape do mixer) ----
    // MIXING: 0 = so deck A, 1 = so deck B, 0.5 = mistura equilibrada.
    mixingFader.setRange (0.0, 1.0, 0.01);
    mixingFader.setValue (0.5);
    mixingFader.setSliderStyle (juce::Slider::LinearVertical);
    mixingFader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    mixingFader.addListener (this);
    addAndMakeVisible (mixingFader);

    // LEVEL: volume geral (0 = silencio, 1 = maximo).
    levelFader.setRange (0.0, 1.0, 0.01);
    levelFader.setValue (0.7);
    levelFader.setSliderStyle (juce::Slider::LinearVertical);
    levelFader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    levelFader.addListener (this);
    addAndMakeVisible (levelFader);

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
    // MIXING: 0 = so deck A, 1 = so deck B, 0.5 = mistura equilibrada.
    // Aplica como canal de cada deck (balanco), nao como crossfader principal.
    if (slider == &mixingFader)
    {
        const auto t = static_cast<double> (mixingFader.getValue());
        const auto gainA = static_cast<float> (std::cos (t * juce::MathConstants<double>::halfPi));
        const auto gainB = static_cast<float> (std::sin (t * juce::MathConstants<double>::halfPi));
        if (playerA != nullptr)
            playerA->setChannelGain (gainA);
        if (playerB != nullptr)
            playerB->setChannelGain (gainB);
        repaint();
        return;
    }

    // LEVEL: volume geral (ganho linear aplicado em ambos os decks).
    if (slider == &levelFader)
    {
        const auto gain = static_cast<float> (levelFader.getValue());
        if (playerA != nullptr)
            playerA->setChannelGain (gain);
        if (playerB != nullptr)
            playerB->setChannelGain (gain);
        repaint();
    }
}

//==============================================================================
void MixerComponent::paint (juce::Graphics& g)
{
    g.fillAll (RekordboxColours::mixerBackground);
}

//==============================================================================
void MixerComponent::resized()
{
    auto area = getLocalBounds();

    // Rodape com MIXING e LEVEL (~72 px).
    auto footer = area.removeFromBottom (juce::jmin (72, area.getHeight() / 4));

    // Os dois canais dividem o resto: 45% cada.
    auto stripArea = area;
    const auto stripWidth = stripArea.getWidth() / 2;

    channelA.setBounds (stripArea.removeFromLeft (stripWidth));
    channelB.setBounds (stripArea.removeFromLeft (stripWidth));

    // O crossfader fica no rodape, entre MIXING e LEVEL.
    auto footerArea = footer;

    // Layout do rodape: MIXING (esquerda) | Crossfader (centro, largo) | LEVEL (direita).
    // MIXING e LEVEL sao faders verticais finos nas laterais; o crossfader
    // horizontal ocupa todo o centro sobrante.
    const auto sideWidth = juce::jmin (56, footerArea.getWidth() / 4);

    auto mixingArea = footerArea.removeFromLeft (sideWidth);
    auto levelArea  = footerArea.removeFromRight (sideWidth);

    // O crossfader horizontal ocupa TODO o centro sobrante do rodape.
    crossfader.setBounds (footerArea.reduced (4));

    // MIXING: fader vertical na lateral esquerda.
    const auto faderWidth = juce::jmin (24, mixingArea.getHeight() - 8);
    mixingFader.setBounds (mixingArea.removeFromLeft (faderWidth)
                           .withSizeKeepingCentre (faderWidth, mixingArea.getHeight() - 4));

    // LEVEL: fader vertical na lateral direita.
    levelFader.setBounds (levelArea.removeFromRight (faderWidth)
                          .withSizeKeepingCentre (faderWidth, levelArea.getHeight() - 4));
}
