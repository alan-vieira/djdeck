#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Knob rotativo com indicador.
//
// Usado para TRIM, HIGH, MID, LOW e FILTER.
//
// O knob e bipolar por padrao: o valor 0 (o neutro) ocupa o angulo 0 (12h) e
// o arco de valor cresce para os dois lados. Um knob unipolar (MIXING, LEVEL)
// comeca no minimo da faixa.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §6.1.
class KnobComponent : public juce::Slider
{
public:
    explicit KnobComponent (juce::String labelToUse);

    void paint (juce::Graphics& g) override;

    // Rotulo desenhado abaixo do knob. Vazio = sem rotulo.
    void setCaption (const juce::String& text);

    // Bipolar: o neutro (0) fica no centro da faixa, no angulo 0 (12h).
    // Unipolar: o neutro e o minimo da faixa.
    void setBipolar (bool shouldBeBipolar);

    // Cor do arco de valor.
    void setAccentColour (const juce::Colour& colour);

    // Angulo do indicador, em radianos, com 0 no topo e sentido horario.
    // O mesmo sistema do setRotaryParameters() do Slider.
    //
    // Nao e const porque valueToProportionOfLength() nao e um metodo const
    // no JUCE 9.
    float getRotaryAngle();

private:
    juce::String caption;
    bool bipolar = true;
    juce::Colour accentColour;

    static constexpr float startAngle = -juce::MathConstants<float>::pi * 0.75f;
    static constexpr float endAngle   =  juce::MathConstants<float>::pi * 0.75f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobComponent)
};
