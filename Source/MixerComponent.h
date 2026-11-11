#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ChannelStripComponent.h"
#include "CrossfaderComponent.h"
#include "DeckPlayer.h"
#include "KnobComponent.h"

//==============================================================================
// Mixer central: dois canais + crossfader.
//
//   ┌─────────┬─────────────────────┬─────────┐
//   │ canal A │    crossfader        │ canal B │
//   │  (strip)│  + MIXING / LEVEL    │ (strip) │
//   └─────────┴─────────────────────┴─────────┘
//
// O mixer NAO tem controle de ganho por fonte no juce::MixerAudioSource —
// por isso o crossfader e aplicado dentro de cada DeckPlayer, antes da soma.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §5.
class MixerComponent : public juce::Component,
                       public juce::Slider::Listener
{
public:
    MixerComponent (DeckPlayer* playerA, DeckPlayer* playerB);
    ~MixerComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Crossfader: -1.0 = deck A no maximo, +1.0 = deck B no maximo.
    void setCrossfaderPosition (double position);
    double getCrossfaderPosition() const { return crossfader.getPosition(); }

private:
    // Aplica os ganhos do crossfader nos dois decks.
    void applyCrossfaderGains();

    // Slider::Listener — MIXING e LEVEL knobs.
    void sliderValueChanged (juce::Slider* slider) override;

    ChannelStripComponent channelA;
    ChannelStripComponent channelB;

    CrossfaderComponent crossfader;

    // MIXING: escolhe o que se ouve no fone (0 = deck A, 1 = deck B,
    // 0.5 = mistura). LEVEL: volume geral do fone.
    //
    // Ambos sao knobs UNIPOLARES: o valor neutro e o centro do arco, mas o
    // caminho e so para um lado.
    KnobComponent mixingKnob { "MIXING" };
    KnobComponent levelKnob  { "LEVEL" };

    DeckPlayer* playerA;
    DeckPlayer* playerB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerComponent)
};
