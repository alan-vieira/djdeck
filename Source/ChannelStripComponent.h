#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DeckPlayer.h"
#include "KnobComponent.h"
#include "VUMeterComponent.h"

//==============================================================================
// Canal do mixer (ChannelStrip).
//
// ESTRUTURA (definida pelo usuario):
//
//   ┌────────────────────────────┐
//   │ VU │  TRIM    (knob)       │
//   │ M  │  HIGH    (knob)       │
//   │ E  │  MID     (knob)       │
//   │ T  │  LOW     (knob)       │
//   │ R  │  FILTER  (knob)       │
//   │    │  [ CUE ] (botao)      │
//   │    │  |volume| (slider)    │
//   └────────────────────────────┘
//
// O VU fica AO LADO dos knobs, numa coluna estreita — nao acima.
//
// REGRA: este componente nao toca em audio. Ele so chama setters do
// DeckPlayer. Ver docs/DOCUMENTACAO_LAYOUT_UI.md §9.2.
class ChannelStripComponent : public juce::Component,
                              public juce::Slider::Listener
{
public:
    // `mirrored` inverte a posicao dos rotulos: a direita no deck 1, a
    // esquerda no deck 2. Nenhum `if (mirrored)` espalhado no desenho — o
    // componente se posiciona a partir do parametro.
    ChannelStripComponent (DeckPlayer* playerToUse, bool mirroredToUse);
    ~ChannelStripComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Slider::Listener
    void sliderValueChanged (juce::Slider* slider) override;

    // Estado do botao CUE (PFL). A implementacao real de PFL exige uma
    // segunda saida de audio — ver §5.8. Por enquanto o estado e publicado
    // para que o roteamento possa ser ligado depois.
    bool isCueActive() const { return cueButton.getToggleState(); }
    void setCueActive (bool shouldBeActive);

    // Chamado quando o usuario mexe no botao CUE.
    std::function<void (bool)> onCueToggled;

private:
    DeckPlayer* player;
    bool mirrored;

    KnobComponent trimKnob { "TRIM" };
    KnobComponent highKnob { "HIGH" };
    KnobComponent midKnob  { "MID" };
    KnobComponent lowKnob  { "LOW" };
    KnobComponent filterKnob { "FILTER" };

    juce::TextButton cueButton { "CUE" };

    // Fader de volume do canal. Range LINEAR 0-1: mantem a API de
    // DeckPlayer::setGain() sem conversao extra.
    juce::Slider volumeFader { juce::Slider::LinearVertical, juce::Slider::NoTextBox };

    // O VU le o nivel publicado pelo DeckPlayer. O DeckPlayer e destruido
    // DEPOIS dos componentes de UI, entao a referencia continua valida.
    VUMeterComponent vuMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStripComponent)
};
