#include "GlobalHeaderComponent.h"
#include "RekordboxColours.h"

//==============================================================================
GlobalHeaderComponent::GlobalHeaderComponent()
{
    startTimer (1000);   // relogio, 1 Hz
}

GlobalHeaderComponent::~GlobalHeaderComponent()
{
    stopTimer();
}

//==============================================================================
void GlobalHeaderComponent::setMode (const juce::String& newMode)
{
    mode = newMode;
    repaint();
}

void GlobalHeaderComponent::setLayoutName (const juce::String& newLayoutName)
{
    layoutName = newLayoutName;
    repaint();
}

//==============================================================================
void GlobalHeaderComponent::timerCallback()
{
    repaint();   // o relogio mudou
}

//==============================================================================
void GlobalHeaderComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (RekordboxColours::panelRaised);

    // Borda inferior: separa o header do resto da tela.
    g.setColour (RekordboxColours::separator);
    g.drawLine (0.0f, static_cast<float> (area.getBottom() - 1),
                static_cast<float> (area.getRight()), static_cast<float> (area.getBottom() - 1), 1.0f);

    // ---- Marca, a esquerda ----
    g.setFont (juce::Font (juce::FontOptions (18.0f)));
    g.setColour (RekordboxColours::textPrimary);
    g.drawText ("djdeck", area.removeFromLeft (100), juce::Justification::centredLeft, true);

    // ---- Modo e layout ----
    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.setColour (RekordboxColours::accent);
    g.drawText (mode, area.removeFromLeft (110), juce::Justification::centredLeft, true);

    g.setColour (RekordboxColours::textSecondary);
    g.drawText (layoutName, area.removeFromLeft (150), juce::Justification::centredLeft, true);

    // ---- Modulos ativos: pips coloridos ----
    auto modulesArea = area.removeFromLeft (220);

    g.setFont (juce::Font (juce::FontOptions (11.0f)));

    for (const auto& module : activeModules)
    {
        const auto pipY = static_cast<float> (modulesArea.getCentreY()) - 3.0f;

        g.setColour (RekordboxColours::accent);
        g.fillEllipse (static_cast<float> (modulesArea.getX()), pipY, 6.0f, 6.0f);

        g.setColour (RekordboxColours::textTertiary);
        g.drawText (module, modulesArea.removeFromLeft (70).withTrimmedRight (8),
                    juce::Justification::centredLeft, true);
    }

    // ---- Relogio, a direita ----
    g.setFont (juce::Font (juce::FontOptions (16.0f)));
    g.setColour (RekordboxColours::textPrimary);

    const auto now = juce::Time::getCurrentTime();
    const auto clock = juce::String (now.getHours()).paddedLeft ('0', 2) + ":"
                     + juce::String (now.getMinutes()).paddedLeft ('0', 2);

    g.drawText (clock, area.removeFromRight (70), juce::Justification::centred, true);
}

//==============================================================================
void GlobalHeaderComponent::resized()
{
    // Layout todo no paint(): o header e uma faixa simples, sem filhos
    // interativos.
}
