#include "HotCuePadsComponent.h"
#include "RekordboxColours.h"

//==============================================================================
HotCuePadsComponent::HotCuePadsComponent()
{
    for (int i = 0; i < numPads; ++i)
    {
        cueTimes[i] = -1.0;

        padButtons[i] = std::make_unique<juce::TextButton> (juce::String (static_cast<char> ('A' + i)));
        padButtons[i]->setClickingTogglesState (false);
        padButtons[i]->addListener (this);

        // A cor de cada pad e definida no proprio botao.
        padButtons[i]->setColour (juce::TextButton::buttonOnColourId, RekordboxColours::hotCue[i]);
        padButtons[i]->setColour (juce::TextButton::buttonColourId,  RekordboxColours::hotCue[i]);
        padButtons[i]->setColour (juce::TextButton::textColourOffId, juce::Colours::black);

        addAndMakeVisible (*padButtons[i]);
    }
}

HotCuePadsComponent::~HotCuePadsComponent() = default;

//==============================================================================
void HotCuePadsComponent::setCueTime (int padIndex, double seconds)
{
    if (padIndex < 0 || padIndex >= numPads)
        return;

    cueTimes[padIndex] = seconds;

    // Atualiza o rotulo do pad com o timestamp.
    const auto letter = juce::String (static_cast<char> ('A' + padIndex));

    if (seconds < 0.0)
    {
        padButtons[padIndex]->setButtonText (letter);
    }
    else
    {
        const auto minutes = static_cast<int> (seconds) / 60;
        const auto secs = seconds - minutes * 60;

        // "1:23.5" — algarismos tabulares para nao tremer durante a reproducao.
        const auto secondsText = juce::String (secs, 1).paddedLeft (' ', 4);

        padButtons[padIndex]->setButtonText (letter + "\n"
                                            + juce::String (minutes) + ":"
                                            + secondsText);
    }

    repaint();
}

//==============================================================================
double HotCuePadsComponent::getCueTime (int padIndex) const
{
    if (padIndex < 0 || padIndex >= numPads)
        return -1.0;

    return cueTimes[padIndex];
}

//==============================================================================
void HotCuePadsComponent::buttonClicked (juce::Button* button)
{
    for (int i = 0; i < numPads; ++i)
    {
        if (padButtons[i].get() != button)
            continue;

        // Um pad vazio ARMA um cue novo; um pad com cue SALTA para ele.
        const auto isSetting = cueTimes[i] < 0.0;

        if (onCueClicked != nullptr)
            onCueClicked (i, cueTimes[i], isSetting);

        return;
    }
}

//==============================================================================
void HotCuePadsComponent::paint (juce::Graphics& g)
{
    g.fillAll (RekordboxColours::panel);
}

//==============================================================================
void HotCuePadsComponent::resized()
{
    auto area = getLocalBounds();

    // Grade de 2 colunas x 4 linhas:
    //
    //   ┌────┬────┐
    //   │ A  │ E  │
    //   ├────┼────┤
    //   │ B  │ F  │
    //   ├────┼────┤
    //   │ C  │ G  │
    //   ├────┼────┤
    //   │ D  │ H  │
    //   └────┴────┘
    //
    // Coluna 0 = pads A-D (de cima para baixo), coluna 1 = pads E-H.
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
