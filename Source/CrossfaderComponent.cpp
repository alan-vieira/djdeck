#include "CrossfaderComponent.h"
#include "RekordboxColours.h"

//==============================================================================
CrossfaderComponent::CrossfaderComponent()
{
    // Posicao: -1.0 (deck A) .. +1.0 (deck B). Centro = mistura equilibrada.
    crossfader.setRange (-1.0, 1.0, 0.001);
    crossfader.setValue (0.0);
    crossfader.addListener (this);
    addAndMakeVisible (crossfader);

    assignOne.setClickingTogglesState (true);
    assignOne.setToggleState (true, juce::dontSendNotification);
    assignOne.setTooltip ("Deck 1 atribuido a este lado do crossfader");
    addAndMakeVisible (assignOne);

    assignTwo.setClickingTogglesState (true);
    assignTwo.setTooltip ("Deck 2 atribuido a este lado do crossfader");
    addAndMakeVisible (assignTwo);

    computeGains (0.0, gainA, gainB);
}

CrossfaderComponent::~CrossfaderComponent() = default;

//==============================================================================
void CrossfaderComponent::computeGains (double position, float& gainA, float& gainB)
{
    // Normaliza para 0..1.
    const auto t = juce::jlimit (0.0, 1.0, (position + 1.0) * 0.5);

    // Constant power: as duas somam em potencia constante.
    gainA = static_cast<float> (std::cos (t * juce::MathConstants<double>::halfPi));
    gainB = static_cast<float> (std::sin (t * juce::MathConstants<double>::halfPi));
}

//==============================================================================
void CrossfaderComponent::setPosition (double newPosition)
{
    position = juce::jlimit (-1.0, 1.0, newPosition);
    crossfader.setValue (position, juce::dontSendNotification);

    computeGains (position, gainA, gainB);

    if (onGainsChanged != nullptr)
        onGainsChanged (gainA, gainB);

    repaint();
}

//==============================================================================
void CrossfaderComponent::sliderValueChanged (juce::Slider* slider)
{
    if (slider != &crossfader)
        return;

    position = juce::jlimit (-1.0, 1.0, crossfader.getValue());
    computeGains (position, gainA, gainB);

    if (onGainsChanged != nullptr)
        onGainsChanged (gainA, gainB);

    repaint();
}

//==============================================================================
void CrossfaderComponent::paint (juce::Graphics& g)
{
    g.fillAll (RekordboxColours::mixerBackground);

    // Rotulos A e B embaixo do fader.
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.setColour (RekordboxColours::textTertiary);

    auto labelArea = getLocalBounds().removeFromBottom (14);
    labelArea.removeFromLeft (labelArea.getWidth() / 2);
    g.drawText ("A", labelArea, juce::Justification::centred, true);

    labelArea = getLocalBounds().removeFromBottom (14);
    labelArea.removeFromRight (labelArea.getWidth() / 2);
    g.drawText ("B", labelArea, juce::Justification::centred, true);
}

//==============================================================================
void CrossfaderComponent::resized()
{
    auto area = getLocalBounds();

    // Faixa dos botoes de atribuicao, acima do fader.
    auto buttonRow = area.removeFromTop (juce::jmin (24, area.getHeight() / 4));

    const auto buttonWidth = juce::jmin (32, buttonRow.getWidth() / 4);
    assignOne.setBounds (buttonRow.removeFromLeft (buttonWidth));
    assignTwo.setBounds (buttonRow.removeFromLeft (buttonWidth));

    // O fader ocupa o resto, com folga para os rotulos A/B.
    auto faderArea = area;
    faderArea.removeFromBottom (14);

    crossfader.setBounds (faderArea.reduced (4));
}
