#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "RekordboxColours.h"

//==============================================================================
// LookAndFeel unico do djdeck.
//
// Todos os widgets visuais sao desenhados por aqui. O LookAndFeel e aplicado
// UMA vez no MainComponent e vale para todos os filhos.
//
// NOTA SOBRE A ORDEM DE DECLARACAO: este objeto e destruido na ordem inversa
// da declaracao dos members. Como os widgets referenciam o LookAndFeel
// enquanto pintam, ele DEVE ser declarado como ULTIMO member do
// MainComponent — caso contrario e destruido antes deles e o app quebra ao
// fechar. Ver docs/DOCUMENTACAO_LAYOUT_UI.md §6.
class RekordboxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RekordboxLookAndFeel();

    // Sliders
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    int getSliderThumbRadius (juce::Slider& slider) override;

    // Botoes
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override;

    juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;

    // Tipografia
    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

private:
    // Desenha o knob rotativo de um Slider. Compartilhado entre
    // drawRotarySlider() e o KnobComponent, para que ambos os knobs tenham
    // exatamente o mesmo desenho.
    void drawKnob (juce::Graphics& g, juce::Rectangle<float> area,
                   float proportion, float startAngle, float endAngle,
                   juce::Colour fillColour);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RekordboxLookAndFeel)
};
