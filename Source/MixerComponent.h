#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ChannelStripComponent.h"
#include "CrossfaderComponent.h"
#include "DeckPlayer.h"
#include "KnobComponent.h"
#include "VUMeterComponent.h"

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
    // Slider::Listener — MIXING e LEVEL faders.
    void sliderValueChanged (juce::Slider* slider) override;

    // Canais (apenas para áudio, sem desenho)
    ChannelStripComponent channelA;
    ChannelStripComponent channelB;

    CrossfaderComponent crossfader;

    DeckPlayer* playerA;
    DeckPlayer* playerB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerComponent)
};
