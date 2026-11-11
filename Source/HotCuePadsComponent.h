#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Grade de 8 pads (4 colunas x 2 linhas) do deck.
//
// Cada pad mostra a LETRA e, quando definido, o TIMESTAMP da posicao de cue.
//
// As cores vem de RekordboxColours::hotCue[8]. A cor e definida NO PROPRIO
// botao — o LookAndFeel global nao distingue um pad de outro.
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
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §6.5.
class HotCuePadsComponent : public juce::Component,
                            public juce::Button::Listener
{
public:
    HotCuePadsComponent();
    ~HotCuePadsComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Button::Listener
    void buttonClicked (juce::Button* button) override;

    // Define a posicao (em segundos) do cue de um pad. Negativo = sem cue.
    void setCueTime (int padIndex, double seconds);
    double getCueTime (int padIndex) const;

    // Chamado quando o usuario clica num pad. `isSetting` e true quando o
    // clique ARMA um cue novo (pad ainda vazio) e false quando o clique
    // SALTA para um cue ja existente.
    std::function<void (int padIndex, double seconds, bool isSetting)> onCueClicked;

    // Numero de pads.
    static constexpr int numPads = 8;

private:
    std::unique_ptr<juce::TextButton> padButtons[numPads];
    double cueTimes[numPads];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HotCuePadsComponent)
};
