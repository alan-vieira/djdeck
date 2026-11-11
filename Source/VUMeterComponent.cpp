#include "VUMeterComponent.h"
#include "RekordboxColours.h"

#include <juce_audio_basics/utilities/juce_Decibels.h>

//==============================================================================
VUMeterComponent::VUMeterComponent (juce::Atomic<float>& sourceLevelDbToUse)
    : sourceLevelDb (sourceLevelDbToUse)
{
    startTimer (33);   // ~30 fps
}

VUMeterComponent::~VUMeterComponent()
{
    stopTimer();
}

void VUMeterComponent::setEnableClipDetection (bool shouldEnable)
{
    enableClipDetection = shouldEnable;
}

//==============================================================================
void VUMeterComponent::timerCallback()
{
    const auto incoming = sourceLevelDb.get();

    // Decay no DOMINIO LINEAR (ganho), nao em dB. Multiplicar dB por 0.8
    // fica mais alto (menos negativo), o que e o oposto de decay.
    const auto incomingGain = juce::Decibels::decibelsToGain(incoming);
    currentGain = juce::jmax(incomingGain, currentGain * 0.8f);
    currentLevelDb = juce::Decibels::gainToDecibels(currentGain);

    if (enableClipDetection && incoming > -0.1f)
        clipDetected = true;
    else if (currentLevelDb < -6.0f)
        clipDetected = false;

    repaint();
}

//==============================================================================
void VUMeterComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    // Fundo
    g.setColour (RekordboxColours::canvas);
    g.fillRect (area);

    constexpr float minDb = -60.0f;
    constexpr float maxDb =   0.0f;

    // Normaliza o nivel para a altura, com o minimo no fundo.
    const auto normalised = juce::jlimit (0.0f, 1.0f,
                                (currentLevelDb - minDb) / (maxDb - minDb));

    const auto fillHeight = static_cast<int> (normalised * area.getHeight());

    if (fillHeight > 0)
    {
        // Gradiente verde -> amarelo -> vermelho, de baixo para cima.
        juce::ColourGradient gradient (juce::Colours::green, 0.0f, static_cast<float> (area.getBottom()),
                                       juce::Colours::red,   0.0f, static_cast<float> (area.getY()), false);
        gradient.addColour (0.75, juce::Colours::yellow);

        g.setGradientFill (gradient);
        g.fillRect (area.getX(), area.getBottom() - fillHeight,
                    area.getWidth(), fillHeight);

        // Limpa o gradiente: a escala e o LED usam cores solidas.
        g.setColour (juce::Colours::transparentBlack);
    }

    // LED de clip
    g.setColour (clipDetected ? RekordboxColours::cue : RekordboxColours::canvas);
    g.fillRect (area.removeFromTop (4));

    drawScale (g, area);
}

//==============================================================================
void VUMeterComponent::drawScale (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    const float marks[] = { 0.0f, -3.0f, -6.0f, -10.0f, -16.0f, -22.0f, -32.0f, -48.0f };
    constexpr float minDb = -60.0f;

    g.setFont (juce::Font (juce::FontOptions (8.0f)));
    g.setColour (RekordboxColours::textTertiary);

    for (const auto mark : marks)
    {
        const auto y = area.getBottom() - (mark / minDb) * area.getHeight();

        g.drawLine (static_cast<float> (area.getRight() - 4), static_cast<float> (y),
                    static_cast<float> (area.getRight()), static_cast<float> (y), 1.0f);
        g.drawText (juce::String (static_cast<int> (mark)),
                    area.getX(), static_cast<int> (y) - 6,
                    area.getWidth() - 6, 12, juce::Justification::topLeft, false);
    }
}
