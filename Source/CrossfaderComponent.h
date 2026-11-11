#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Crossfader horizontal, entre os dois canais do mixer.
//
//   ┌─────────────────────────────────────────┐
//   │  [1]  [2]        <- atribuicao ao crossfader
//   │                                         │
//   │  ---------[*]------------                │  <- fader horizontal
//   │            A        B                   │
//   └─────────────────────────────────────────┘
//
// Os botoes 1 e 2 sao ATRIBUICAO ao crossfader (a qual lado cada deck e
// atribuido), nao selecao de deck.
//
// A curva e CONSTANT POWER: preserva a energia percebida na saida. No centro,
// cada deck fica a -3 dB (0.707) com a mesma energia total.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §5.10.
class CrossfaderComponent : public juce::Component,
                             public juce::Slider::Listener
{
public:
    CrossfaderComponent();
    ~CrossfaderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Slider::Listener
    void sliderValueChanged (juce::Slider* slider) override;

    // Posicao do crossfader: -1.0 = deck A no maximo, +1.0 = deck B no maximo.
    void setPosition (double position);
    double getPosition() const { return position; }

    // Calcula o par de ganhos (deckA, deckB) em escala linear.
    //
    // Constant power: gainA = cos(t·π/2), gainB = sin(t·π/2).
    // No centro (t = 0.5): 0.707 cada — -3 dB, energia constante.
    static void computeGains (double position, float& gainA, float& gainB);

    // Ganhos atuais, para aplicacao pelo MixerComponent.
    float getGainA() const { return gainA; }
    float getGainB() const { return gainB; }

    // Chamado quando o usuario move o crossfader.
    std::function<void (float gainA, float gainB)> onGainsChanged;

private:
    juce::Slider crossfader { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };

    // Botoes de atribuicao ao crossfader.
    juce::TextButton assignOne { "1" };
    juce::TextButton assignTwo { "2" };

    double position = 0.0;
    float gainA = 0.7071f;
    float gainB = 0.7071f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrossfaderComponent)
};
