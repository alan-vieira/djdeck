#include "MixerComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
MixerComponent::MixerComponent (DeckPlayer* playerAToUse, DeckPlayer* playerBToUse)
    : channelA (playerAToUse, false, 0),   // deck 1: offset X = 0 dentro do mixer
      channelB (playerBToUse, true, 54),   // deck 2: offset X = 54 dentro do mixer
      playerA (playerAToUse),
      playerB (playerBToUse)
{
    addAndMakeVisible (channelA);
    addAndMakeVisible (channelB);
    addAndMakeVisible (crossfader);

    // ---- MIXING / LEVEL (rodape do mixer) ----
    mixingFader.setRange (0.0, 1.0, 0.01);
    mixingFader.setValue (0.5);
    mixingFader.setDoubleClickReturnValue (true, 0.5);
    mixingFader.setSliderStyle (juce::Slider::LinearVertical);
    mixingFader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    mixingFader.addListener (this);
    addAndMakeVisible (mixingFader);

    levelFader.setRange (0.0, 1.0, 0.01);
    levelFader.setValue (0.7);
    levelFader.setDoubleClickReturnValue (true, 0.7);
    levelFader.setSliderStyle (juce::Slider::LinearVertical);
    levelFader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    levelFader.addListener (this);
    addAndMakeVisible (levelFader);

    // ---- Knobs do deck 1 ----
    knobTrim1.setRange (-12.0f, 12.0f, 0.1f);
    knobTrim1.setValue (0.0f);
    knobTrim1.setDoubleClickReturnValue (true, 0.0);
    knobTrim1.addListener (this);
    addAndMakeVisible (knobTrim1);

    knobHigh1.setRange (-12.0f, 12.0f, 0.5f);
    knobHigh1.setValue (0.0f);
    knobHigh1.setDoubleClickReturnValue (true, 0.0);
    knobHigh1.addListener (this);
    addAndMakeVisible (knobHigh1);

    knobMid1.setRange (-12.0f, 12.0f, 0.5f);
    knobMid1.setValue (0.0f);
    knobMid1.setDoubleClickReturnValue (true, 0.0);
    knobMid1.addListener (this);
    addAndMakeVisible (knobMid1);

    knobLow1.setRange (-12.0f, 12.0f, 0.5f);
    knobLow1.setValue (0.0f);
    knobLow1.setDoubleClickReturnValue (true, 0.0);
    knobLow1.addListener (this);
    addAndMakeVisible (knobLow1);

    knobFilter1.setRange (-1.0f, 1.0f, 0.01f);
    knobFilter1.setValue (0.0f);
    knobFilter1.setDoubleClickReturnValue (true, 0.0);
    knobFilter1.addListener (this);
    addAndMakeVisible (knobFilter1);

    // ---- Knobs do deck 2 ----
    knobTrim2.setRange (-12.0f, 12.0f, 0.1f);
    knobTrim2.setValue (0.0f);
    knobTrim2.setDoubleClickReturnValue (true, 0.0);
    knobTrim2.addListener (this);
    addAndMakeVisible (knobTrim2);

    knobHigh2.setRange (-12.0f, 12.0f, 0.5f);
    knobHigh2.setValue (0.0f);
    knobHigh2.setDoubleClickReturnValue (true, 0.0);
    knobHigh2.addListener (this);
    addAndMakeVisible (knobHigh2);

    knobMid2.setRange (-12.0f, 12.0f, 0.5f);
    knobMid2.setValue (0.0f);
    knobMid2.setDoubleClickReturnValue (true, 0.0);
    knobMid2.addListener (this);
    addAndMakeVisible (knobMid2);

    knobLow2.setRange (-12.0f, 12.0f, 0.5f);
    knobLow2.setValue (0.0f);
    knobLow2.setDoubleClickReturnValue (true, 0.0);
    knobLow2.addListener (this);
    addAndMakeVisible (knobLow2);

    knobFilter2.setRange (-1.0f, 1.0f, 0.01f);
    knobFilter2.setValue (0.0f);
    knobFilter2.setDoubleClickReturnValue (true, 0.0);
    knobFilter2.addListener (this);
    addAndMakeVisible (knobFilter2);

    // ---- Botões CUE ----
    cueBtn1.setClickingTogglesState (true);
        cueBtn1.setTooltip ("Monitoramento de fone (PFL)");
        cueBtn1.setColour (juce::TextButton::buttonColourId, djdeck::layout::colours::CueColor);
        cueBtn1.setColour (juce::TextButton::textColourOffId, djdeck::layout::colours::TextPrimary);
        cueBtn1.setColour (juce::TextButton::textColourOnId, djdeck::layout::colours::TextPrimary);
        addAndMakeVisible (cueBtn1);

        cueBtn2.setClickingTogglesState (true);
        cueBtn2.setTooltip ("Monitoramento de fone (PFL)");
        cueBtn2.setColour (juce::TextButton::buttonColourId, djdeck::layout::colours::CueColor);
        cueBtn2.setColour (juce::TextButton::textColourOffId, djdeck::layout::colours::TextPrimary);
        cueBtn2.setColour (juce::TextButton::textColourOnId, djdeck::layout::colours::TextPrimary);
        addAndMakeVisible (cueBtn2);

    // O crossfader publica os ganhos; aplicamos nos DeckPlayers.
    crossfader.onGainsChanged = [this] (float gainA, float gainB)
    {
        if (playerA != nullptr)
            playerA->setCrossfadeGain (gainA);
        if (playerB != nullptr)
            playerB->setCrossfadeGain (gainB);
    };
}

MixerComponent::~MixerComponent() = default;

//==============================================================================
void MixerComponent::setCrossfaderPosition (double position)
{
    crossfader.setPosition (position);
}

//==============================================================================
void MixerComponent::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &mixingFader)
    {
        const auto t = static_cast<double> (mixingFader.getValue());
        const auto gainA = static_cast<float> (std::cos (t * juce::MathConstants<double>::halfPi));
        const auto gainB = static_cast<float> (std::sin (t * juce::MathConstants<double>::halfPi));
        if (playerA != nullptr)
            playerA->setChannelGain (gainA);
        if (playerB != nullptr)
            playerB->setChannelGain (gainB);
        repaint();
        return;
    }

    if (slider == &levelFader)
    {
        const auto gain = static_cast<float> (levelFader.getValue());
        if (playerA != nullptr)
            playerA->setChannelGain (gain);
        if (playerB != nullptr)
            playerB->setChannelGain (gain);
        repaint();
    }

    // Knobs do deck 1
    if (slider == &knobTrim1)
        playerA->setTrimDb (knobTrim1.getValue());
    else if (slider == &knobHigh1)
        playerA->setEqHighDb (knobHigh1.getValue());
    else if (slider == &knobMid1)
        playerA->setEqMidDb (knobMid1.getValue());
    else if (slider == &knobLow1)
        playerA->setEqLowDb (knobLow1.getValue());
    else if (slider == &knobFilter1)
        playerA->setFilterAmount (knobFilter1.getValue());

    // Knobs do deck 2
    else if (slider == &knobTrim2)
        playerB->setTrimDb (knobTrim2.getValue());
    else if (slider == &knobHigh2)
        playerB->setEqHighDb (knobHigh2.getValue());
    else if (slider == &knobMid2)
        playerB->setEqMidDb (knobMid2.getValue());
    else if (slider == &knobLow2)
        playerB->setEqLowDb (knobLow2.getValue());
    else if (slider == &knobFilter2)
        playerB->setFilterAmount (knobFilter2.getValue());
}

//==============================================================================
void MixerComponent::paint (juce::Graphics& g)
{
    g.fillAll (djdeck::layout::colours::Mixerbackground);

    g.setColour (djdeck::layout::colours::Separator);
    g.drawRect (getLocalBounds(), 1);
}

//==============================================================================
void MixerComponent::resized()
{
    // Mixer: x672 y138 l108 a241 (medida do Figma: figmaNode8_1134)
    // Todas as coordenadas são absolutas do canvas do Figma, convertidas
    // para locais do mixer subtraindo (672, 138).
    const int mixerAbsX = 672;
    const int mixerAbsY = 138;

    auto xLocal = [&] (int figmaX)  { return figmaX - mixerAbsX; };
    auto yLocal = [&] (int figmaY)  { return figmaY - mixerAbsY; };
    auto xLocalf = [&] (double figmaX)  { return static_cast<int> (figmaX - mixerAbsX); };
    auto yLocalf = [&] (double figmaY)  { return static_cast<int> (figmaY - mixerAbsY); };

    // ---- Canais (54px cada) ----
    channelA.setBounds (0, djdeck::layout::deckHeader::heightPx,
                        djdeck::layout::figmaNode8_1134::widthPx / 2,
                        djdeck::layout::figmaNode8_1134::heightPx - djdeck::layout::deckHeader::heightPx);
    channelB.setBounds (djdeck::layout::figmaNode8_1134::widthPx / 2,
                        djdeck::layout::deckHeader::heightPx,
                        djdeck::layout::figmaNode8_1134::widthPx / 2,
                        djdeck::layout::figmaNode8_1134::heightPx - djdeck::layout::deckHeader::heightPx);

    // ---- Crossfader button ----
    // Figma: x719 y354 l12 a10  -> relativo ao mixer: x47 y216
    crossfader.setBounds (xLocal (djdeck::layout::figmaNodemixerCrossfaderBtn1::xPx),
                          yLocal (djdeck::layout::figmaNodemixerCrossfaderBtn1::yPx),
                          djdeck::layout::figmaNodemixerCrossfaderBtn1::widthPx,
                          djdeck::layout::figmaNodemixerCrossfaderBtn1::heightPx);

    // ---- Faders de volume ----
    // Figma deck1: trilho x699 y293 l2 a46  -> relativo: x27 y155
    // Figma deck2: trilho x751 y293 l2 a46  -> relativo: x79 y155
    mixingFader.setBounds (xLocal (djdeck::layout::figmaNodemixerVolRail1::xPx),
                           yLocal (djdeck::layout::figmaNodemixerVolRail1::yPx),
                           djdeck::layout::figmaNodemixerVolRail1::widthPx,
                           djdeck::layout::figmaNodemixerVolRail1::heightPx);
    levelFader.setBounds (xLocal (djdeck::layout::figmaNodemixerVolRail2::xPx),
                          yLocal (djdeck::layout::figmaNodemixerVolRail2::yPx),
                          djdeck::layout::figmaNodemixerVolRail2::widthPx,
                          djdeck::layout::figmaNodemixerVolRail2::heightPx);

    // ---- Knobs EQ/Trim do deck 1 ----
    // Figma deck1: knobs em x690
    // trim: y145.04 -> relativo y7
    // high: y178.04 -> relativo y40
    // mid:  y211.04 -> relativo y73
    // low:  y244.04 -> relativo y106
    const int knobW = 20;
    const int knobH = 20;

    knobTrim1.setBounds  (xLocalf (djdeck::layout::figmaNodemixerKnobTrim1::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobTrim1::yPx),
                          knobW, knobH);
    knobHigh1.setBounds  (xLocalf (djdeck::layout::figmaNodemixerKnobHigh1::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobHigh1::yPx),
                          knobW, knobH);
    knobMid1.setBounds   (xLocalf (djdeck::layout::figmaNodemixerKnobMid1::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobMid1::yPx),
                          knobW, knobH);
    knobLow1.setBounds   (xLocalf (djdeck::layout::figmaNodemixerKnobLow1::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobLow1::yPx),
                          knobW, knobH);
    knobFilter1.setBounds(xLocalf (djdeck::layout::figmaNodemixerKnobTrim1::xPx),
                          yLocal (static_cast<int> (djdeck::layout::figmaNodemixerKnobLow1::yPx) + 33),
                          knobW, knobH);

    // ---- Knobs EQ/Trim do deck 2 ----
    // Figma deck2: knobs em x742 -> relativo ao mixer: x70
    knobTrim2.setBounds  (xLocalf (djdeck::layout::figmaNodemixerKnobTrim2::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobTrim2::yPx),
                          knobW, knobH);
    knobHigh2.setBounds  (xLocalf (djdeck::layout::figmaNodemixerKnobHigh2::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobHigh2::yPx),
                          knobW, knobH);
    knobMid2.setBounds   (xLocalf (djdeck::layout::figmaNodemixerKnobMid2::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobMid2::yPx),
                          knobW, knobH);
    knobLow2.setBounds   (xLocalf (djdeck::layout::figmaNodemixerKnobLow2::xPx),
                          yLocalf (djdeck::layout::figmaNodemixerKnobLow2::yPx),
                          knobW, knobH);
    knobFilter2.setBounds(xLocalf (djdeck::layout::figmaNodemixerKnobTrim2::xPx),
                          yLocal (static_cast<int> (djdeck::layout::figmaNodemixerKnobLow2::yPx) + 33),
                          knobW, knobH);

    // ---- Botões CUE ----
    // deck1: x685 y276 l30 a14  -> relativo: x13 y138
    // deck2: x737 y276 l30 a14  -> relativo: x65 y138
    cueBtn1.setBounds (xLocal (djdeck::layout::figmaNodemixerCueBtn1::xPx),
                       yLocal (djdeck::layout::figmaNodemixerCueBtn1::yPx),
                       djdeck::layout::figmaNodemixerCueBtn1::widthPx,
                       djdeck::layout::figmaNodemixerCueBtn1::heightPx);
    cueBtn2.setBounds (xLocal (djdeck::layout::figmaNodemixerCueBtn2::xPx),
                       yLocal (djdeck::layout::figmaNodemixerCueBtn2::yPx),
                       djdeck::layout::figmaNodemixerCueBtn2::widthPx,
                       djdeck::layout::figmaNodemixerCueBtn2::heightPx);
}
