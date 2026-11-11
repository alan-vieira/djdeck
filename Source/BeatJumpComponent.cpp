#include "BeatJumpComponent.h"
#include "RekordboxColours.h"

//==============================================================================
namespace
{
    // Rotulos dos pads, em ordem: negativo (recua), neutro, positivo (avanca).
    const char* const padLabels[BeatJumpComponent::numPads] =
    {
        "-4", "-2", "-1", "-1/2", "1/2", "1", "2", "4"
    };
}

//==============================================================================
BeatJumpComponent::BeatJumpComponent()
{
    for (int i = 0; i < numPads; ++i)
    {
        padButtons[i] = std::make_unique<juce::TextButton> (padLabels[i]);
        padButtons[i]->setClickingTogglesState (false);
        padButtons[i]->addListener (this);

        // Os pads de beat jump usam a cor de destaque; os negativos ficam
        // num tom mais escuro para diferenciar a direcao.
        const auto colour = (i < 4) ? RekordboxColours::panelRaised
                                    : RekordboxColours::accent;

        padButtons[i]->setColour (juce::TextButton::buttonColourId,   colour);
        padButtons[i]->setColour (juce::TextButton::buttonOnColourId, colour);
        padButtons[i]->setColour (juce::TextButton::textColourOffId,
                                  (i < 4) ? RekordboxColours::textSecondary
                                          : RekordboxColours::textPrimary);

        addAndMakeVisible (*padButtons[i]);
    }
}

BeatJumpComponent::~BeatJumpComponent() = default;

//==============================================================================
double BeatJumpComponent::getBeatFraction (int padIndex)
{
    switch (padIndex)
    {
        case 0: return -4.0;
        case 1: return -2.0;
        case 2: return -1.0;
        case 3: return -0.5;
        case 4: return  0.5;
        case 5: return  1.0;
        case 6: return  2.0;
        case 7: return  4.0;
        default: return 0.0;
    }
}

//==============================================================================
void BeatJumpComponent::buttonClicked (juce::Button* button)
{
    for (int i = 0; i < numPads; ++i)
    {
        if (padButtons[i].get() != button)
            continue;

        if (onBeatJump != nullptr)
            onBeatJump (getBeatFraction (i));

        return;
    }
}

//==============================================================================
void BeatJumpComponent::paint (juce::Graphics& g)
{
    g.fillAll (RekordboxColours::panel);
}

//==============================================================================
void BeatJumpComponent::resized()
{
    auto area = getLocalBounds();

    // Mesma grade do Hot Cue: 2 colunas x 4 linhas.
    const auto columnWidth = area.getWidth() / 2;
    const auto rowHeight = area.getHeight() / 4;

    for (int col = 0; col < 2; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            const auto index = col * 4 + row;

            const auto r = juce::Rectangle<int> (area.getX() + col * columnWidth,
                                                 area.getY() + row * rowHeight,
                                                 columnWidth,
                                                 rowHeight);

            padButtons[index]->setBounds (r.reduced (2));
        }
    }
}
