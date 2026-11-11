#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Grade de Beat Jump — mesma estrutura de grade dos hot cue pads, mas com
// os rotulos em FRACOES DE BATIDA (1/8, 1/4, 1/2, 1, 2, 4, 8) em vez de
// letras.
//
// No Rekordbox o Beat Jump fica no deck 1 e o Hot Cue no deck 2.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §6.5.
class BeatJumpComponent : public juce::Component,
                          public juce::Button::Listener
{
public:
    BeatJumpComponent();
    ~BeatJumpComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Button::Listener
    void buttonClicked (juce::Button* button) override;

    // Fracao de batida de cada pad. Positivo = avanca, negativo = recua.
    // O pad central (indice 4) e o neutro: nao move.
    static double getBeatFraction (int padIndex);

    // Chamado quando o usuario clica num pad. `fraction` e a fracao de
    // batida; 0 = neutro.
    std::function<void (double fraction)> onBeatJump;

    static constexpr int numPads = 8;

private:
    std::unique_ptr<juce::TextButton> padButtons[numPads];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatJumpComponent)
};
