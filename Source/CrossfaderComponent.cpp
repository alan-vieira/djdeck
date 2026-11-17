#include "CrossfaderComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
CrossfaderComponent::CrossfaderComponent()
{
    // Posicao: -1.0 (deck A) .. +1.0 (deck B). Centro = mistura equilibrada.
    crossfader.setRange (-1.0, 1.0, 0.001);
    crossfader.setValue (0.0);
    crossfader.setDoubleClickReturnValue (true, 0.0);
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
    g.fillAll (djdeck::layout::colours::Mixerbackground);

    // Rotulos A e B embaixo do fader.
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.setColour (djdeck::layout::colours::Tertiary);

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
    // Botoes 1 e 2: 12 x 10 px (medida do Figma).
    auto buttonRow = area.removeFromTop (djdeck::layout::crossfader::assignButton::heightPx);
    const auto buttonWidth = djdeck::layout::crossfader::assignButton::widthPx;
    const auto buttonHeight = djdeck::layout::crossfader::assignButton::heightPx;
    assignOne.setBounds (buttonRow.removeFromLeft (buttonWidth).withSizeKeepingCentre (buttonWidth, buttonHeight));
    assignTwo.setBounds (buttonRow.removeFromLeft (buttonWidth).withSizeKeepingCentre (buttonWidth, buttonHeight));

    // O fader ocupa o resto, com folga para os rotulos A/B.
    // Trilha do slider: 100 x 2 px (medida do Figma).
    auto faderArea = area;
    faderArea.removeFromBottom (14);
    const auto faderWidth = djdeck::layout::crossfader::rail::widthPx;
    const auto faderHeight = djdeck::layout::crossfader::rail::heightPx;
    crossfader.setBounds (faderArea.getX() + (faderArea.getWidth() - faderWidth) / 2,
                          faderArea.getY() + (faderArea.getHeight() - faderHeight) / 2,
                          faderWidth, faderHeight);
}
