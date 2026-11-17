#include "ChannelStripComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
ChannelStripComponent::ChannelStripComponent (DeckPlayer* playerToUse,
                                              bool mirroredToUse,
                                              int channelOffsetXArg)
    : player (playerToUse),
      mirrored (mirroredToUse),
      channelOffsetX (channelOffsetXArg),
      // `player` e declarado ANTES de `vuMeter` no header, e os members sao
      // inicializados na ordem da declaracao — entao o ponteiro ja vale aqui.
      vuMeter (playerToUse->getLevelSource (0))
{
    // ---- Knobs ----
    trimKnob.setRange (-12.0f, 12.0f, 0.1f);
    trimKnob.setValue (0.0f);
    trimKnob.setDoubleClickReturnValue (true, 0.0);

    highKnob.setRange (-12.0f, 12.0f, 0.5f);
    highKnob.setValue (0.0f);
    highKnob.setDoubleClickReturnValue (true, 0.0);

    midKnob.setRange (-12.0f, 12.0f, 0.5f);
    midKnob.setValue (0.0f);
    midKnob.setDoubleClickReturnValue (true, 0.0);

    lowKnob.setRange (-12.0f, 12.0f, 0.5f);
    lowKnob.setValue (0.0f);
    lowKnob.setDoubleClickReturnValue (true, 0.0);

    filterKnob.setRange (-1.0f, 1.0f, 0.01f);
    filterKnob.setValue (0.0f);
    filterKnob.setDoubleClickReturnValue (true, 0.0);

    addAndMakeVisible (trimKnob);
    addAndMakeVisible (highKnob);
    addAndMakeVisible (midKnob);
    addAndMakeVisible (lowKnob);
    addAndMakeVisible (filterKnob);

    for (auto* knob : { &trimKnob, &highKnob, &midKnob, &lowKnob, &filterKnob })
        knob->addListener (this);

    // ---- Botao CUE ----
    cueButton.setClickingTogglesState (true);
    cueButton.setTooltip ("Monitoramento de fone (PFL)");
    cueButton.setColour (juce::TextButton::buttonColourId, djdeck::layout::colours::CueColor);
    cueButton.setColour (juce::TextButton::textColourOffId, djdeck::layout::colours::TextPrimary);
    cueButton.setColour (juce::TextButton::textColourOnId, djdeck::layout::colours::TextPrimary);
    addAndMakeVisible (cueButton);

    // ---- Slider de volume ----
    volumeFader.setRange (0.0, 1.0);
    volumeFader.setValue (player != nullptr ? player->getGain() : 0.5);
    volumeFader.setDoubleClickReturnValue (true, 0.5);
    volumeFader.addListener (this);
    volumeFader.setColour (juce::Slider::trackColourId, djdeck::layout::colours::VolumeRail);
    volumeFader.setColour (juce::Slider::thumbColourId, djdeck::layout::colours::VolumeButton);
    addAndMakeVisible (volumeFader);

    // ---- VU meter ----
    addAndMakeVisible (vuMeter);
}

ChannelStripComponent::~ChannelStripComponent() = default;

//==============================================================================
void ChannelStripComponent::setCueActive (bool shouldBeActive)
{
    cueButton.setToggleState (shouldBeActive, juce::dontSendNotification);
}

//==============================================================================
void ChannelStripComponent::paint (juce::Graphics& g)
{
    g.fillAll (djdeck::layout::colours::Panelraised);

    g.setColour (djdeck::layout::colours::Separator);
    g.drawRect (getLocalBounds(), 1);
}

//==============================================================================
//==============================================================================
void ChannelStripComponent::resized()
{
    // Coordenadas absolutas do Figma (canvas 1452x813) convertidas para
    // locais deste canal: figmaX - 672 - channelOffsetX, figmaY - 138.
    // Canal 1: channelOffsetX = 0 (ocupa x=0..54 do mixer)
    // Canal 2: channelOffsetX = 54 (ocupa x=54..108 do mixer)
    const int mixerAbsX = 672;
    const int mixerAbsY = 138;

    auto xLocal = [&] (int figmaX)  { return figmaX - mixerAbsX - channelOffsetX; };
    auto yLocal = [&] (int figmaY)  { return figmaY - mixerAbsY; };
    auto xLocalf = [&] (double figmaX)  { return static_cast<int> (figmaX - mixerAbsX - channelOffsetX); };
    auto yLocalf = [&] (double figmaY)  { return static_cast<int> (figmaY - mixerAbsY); };

    // ---- VU meter ----
    // deck1: x680 y145 l2 a118  ; deck2: x732 y145 l1 a118
    vuMeter.setBounds (xLocal (static_cast<int> (djdeck::layout::figmaNodemixerVu1::xPx)),
                       yLocal (static_cast<int> (djdeck::layout::figmaNodemixerVu1::yPx)),
                       djdeck::layout::figmaNodemixerVu1::widthPx,
                       djdeck::layout::figmaNodemixerVu1::heightPx);

    // ---- Knobs (20 x 20 px cada) ----
    const int knobW = 20;
    const int knobH = 20;

    trimKnob.setBounds  (xLocalf (djdeck::layout::figmaNodemixerKnobTrim1::xPx),
                         yLocalf (djdeck::layout::figmaNodemixerKnobTrim1::yPx),
                         knobW, knobH);
    highKnob.setBounds  (xLocalf (djdeck::layout::figmaNodemixerKnobHigh1::xPx),
                         yLocalf (djdeck::layout::figmaNodemixerKnobHigh1::yPx),
                         knobW, knobH);
    midKnob.setBounds   (xLocalf (djdeck::layout::figmaNodemixerKnobMid1::xPx),
                         yLocalf (djdeck::layout::figmaNodemixerKnobMid1::yPx),
                         knobW, knobH);
    lowKnob.setBounds   (xLocalf (djdeck::layout::figmaNodemixerKnobLow1::xPx),
                         yLocalf (djdeck::layout::figmaNodemixerKnobLow1::yPx),
                         knobW, knobH);
    filterKnob.setBounds(xLocalf (djdeck::layout::figmaNodemixerKnobTrim1::xPx),
                         yLocal (static_cast<int> (djdeck::layout::figmaNodemixerKnobLow1::yPx) + 33),
                         knobW, knobH);

    // ---- Botao CUE (30 x 14 px) ----
    cueButton.setBounds (xLocal (djdeck::layout::figmaNodemixerCueBtn1::xPx),
                         yLocal (djdeck::layout::figmaNodemixerCueBtn1::yPx),
                         djdeck::layout::figmaNodemixerCueBtn1::widthPx,
                         djdeck::layout::figmaNodemixerCueBtn1::heightPx);

    // ---- Fader de volume (trilho 2 x 46) ----
    {
        int vx = xLocalf (djdeck::layout::figmaNodemixerVolRail1::xPx);
        int vy = yLocalf (djdeck::layout::figmaNodemixerVolRail1::yPx);
        int vw = djdeck::layout::figmaNodemixerVolRail1::widthPx;
        int vh = djdeck::layout::figmaNodemixerVolRail1::heightPx;
        volumeFader.setBounds (vx, vy, vw, vh);
    }
}

//==============================================================================
void ChannelStripComponent::sliderValueChanged (juce::Slider* slider)
{
    if (player == nullptr)
        return;

    if (slider == &trimKnob)
        player->setTrimDb (trimKnob.getValue());
    else if (slider == &highKnob)
        player->setEqHighDb (highKnob.getValue());
    else if (slider == &midKnob)
        player->setEqMidDb (midKnob.getValue());
    else if (slider == &lowKnob)
        player->setEqLowDb (lowKnob.getValue());
    else if (slider == &filterKnob)
        player->setFilterAmount (filterKnob.getValue());
    else if (slider == &volumeFader)
        player->setGain (volumeFader.getValue());
}
