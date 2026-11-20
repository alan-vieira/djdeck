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
    trimKnob.setRange (-12.0f, 12.0f);
    trimKnob.setValue (0.0f);

    highKnob.setRange (-12.0f, 12.0f);
    highKnob.setValue (0.0f);

    midKnob.setRange (-12.0f, 12.0f);
    midKnob.setValue (0.0f);

    lowKnob.setRange (-12.0f, 12.0f);
    lowKnob.setValue (0.0f);

    addAndMakeVisible (trimKnob);
    addAndMakeVisible (highKnob);
    addAndMakeVisible (midKnob);
    addAndMakeVisible (lowKnob);

    trimKnob.onValueChanged = [this] (float v) { player->setTrimDb (v); };
    highKnob.onValueChanged = [this] (float v) { player->setEqHighDb (v); };
    midKnob.onValueChanged  = [this] (float v) { player->setEqMidDb (v);  };
    lowKnob.onValueChanged  = [this] (float v) { player->setEqLowDb (v);  };

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
    volumeFader.setName ("mixerVolume");
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
    // O canal NAO pinta fundo proprio. No prototipo HTML o mixer e uma
    // unica superficie (#0F0F14) e os controles ficam posicionados por
    // cima; pintar #282828 aqui cobria o fundo do mixer inteiro.
    juce::ignoreUnused (g);
}

//==============================================================================
//==============================================================================
void ChannelStripComponent::resized()
{
    // Coordenadas absolutas do Figma (canvas 1452x813) convertidas para
    // locais do mixer: figmaX - 672 - channelOffsetX, figmaY - 138.
    // Os nos do Figma ja estao em coordenadas absolutas do mixer inteiro,
    // entao subtraimos tambem o deslocamento X deste canal dentro do mixer.
    const int mixerAbsX = 672;
    const int mixerAbsY = 137;

    auto xLocal = [&] (int figmaX)  { return figmaX - mixerAbsX - channelOffsetX; };
    auto yLocal = [&] (int figmaY)  { return figmaY - mixerAbsY; };
    auto xLocalf = [&] (double figmaX)  { return static_cast<int> (figmaX - mixerAbsX - channelOffsetX); };
    auto yLocalf = [&] (double figmaY)  { return static_cast<int> (figmaY - mixerAbsY); };

    // Nos do canal ativo (deck 1 ou deck 2), resolvidos via ternário.
    // Usamos variáveis nao-const (int/auto) para evitar o C3536 do MSVC com ternário const.
    const auto vux = mirrored ? djdeck::layout::figmaNodemixerVu2::xPx  : djdeck::layout::figmaNodemixerVu1::xPx;
    const auto vuy = mirrored ? djdeck::layout::figmaNodemixerVu2::yPx  : djdeck::layout::figmaNodemixerVu1::yPx;
    const auto vuw = mirrored ? djdeck::layout::figmaNodemixerVu2::widthPx : djdeck::layout::figmaNodemixerVu1::widthPx;
    const auto vuh = mirrored ? djdeck::layout::figmaNodemixerVu2::heightPx : djdeck::layout::figmaNodemixerVu1::heightPx;

    const auto k1x = mirrored ? djdeck::layout::figmaNodemixerKnobTrim2::xPx : djdeck::layout::figmaNodemixerKnobTrim1::xPx;
    const auto k1y = mirrored ? djdeck::layout::figmaNodemixerKnobTrim2::yPx : djdeck::layout::figmaNodemixerKnobTrim1::yPx;
    const auto k2x = mirrored ? djdeck::layout::figmaNodemixerKnobHigh2::xPx  : djdeck::layout::figmaNodemixerKnobHigh1::xPx;
    const auto k2y = mirrored ? djdeck::layout::figmaNodemixerKnobHigh2::yPx  : djdeck::layout::figmaNodemixerKnobHigh1::yPx;
    const auto k3x = mirrored ? djdeck::layout::figmaNodemixerKnobMid2::xPx   : djdeck::layout::figmaNodemixerKnobMid1::xPx;
    const auto k3y = mirrored ? djdeck::layout::figmaNodemixerKnobMid2::yPx   : djdeck::layout::figmaNodemixerKnobMid1::yPx;
    const auto k4x = mirrored ? djdeck::layout::figmaNodemixerKnobLow2::xPx   : djdeck::layout::figmaNodemixerKnobLow1::xPx;
    const auto k4y = mirrored ? djdeck::layout::figmaNodemixerKnobLow2::yPx   : djdeck::layout::figmaNodemixerKnobLow1::yPx;

    const auto cux = mirrored ? djdeck::layout::figmaNodemixerCueBtn2::xPx : djdeck::layout::figmaNodemixerCueBtn1::xPx;
    const auto cuy = mirrored ? djdeck::layout::figmaNodemixerCueBtn2::yPx : djdeck::layout::figmaNodemixerCueBtn1::yPx;
    const auto cuw = mirrored ? djdeck::layout::figmaNodemixerCueBtn2::widthPx : djdeck::layout::figmaNodemixerCueBtn1::widthPx;
    const auto cuh = mirrored ? djdeck::layout::figmaNodemixerCueBtn2::heightPx : djdeck::layout::figmaNodemixerCueBtn1::heightPx;

    const auto vfx = mirrored ? djdeck::layout::figmaNodemixerVolRail2::xPx : djdeck::layout::figmaNodemixerVolRail1::xPx;
    const auto vfy = mirrored ? djdeck::layout::figmaNodemixerVolRail2::yPx : djdeck::layout::figmaNodemixerVolRail1::yPx;
    const auto vfw = mirrored ? djdeck::layout::figmaNodemixerVolRail2::widthPx : djdeck::layout::figmaNodemixerVolRail1::widthPx;
    const auto vfh = mirrored ? djdeck::layout::figmaNodemixerVolRail2::heightPx : djdeck::layout::figmaNodemixerVolRail1::heightPx;

    // ---- VU meter ----
    vuMeter.setBounds (xLocal (static_cast<int> (vux)), yLocal (static_cast<int> (vuy)),
                       static_cast<int> (vuw), static_cast<int> (vuh));

    // ---- Knobs (conforme layout.json) ----
    const int knobW = djdeck::layout::figmaNodemixerKnobTrim1::widthPx;
    const int knobH = djdeck::layout::figmaNodemixerKnobTrim1::heightPx;

    trimKnob.setBounds  (xLocalf (k1x), yLocalf (k1y), knobW, knobH);
    highKnob.setBounds  (xLocalf (k2x), yLocalf (k2y), knobW, knobH);
    midKnob.setBounds   (xLocalf (k3x), yLocalf (k3y), knobW, knobH);
    lowKnob.setBounds   (xLocalf (k4x), yLocalf (k4y), knobW, knobH);

    // ---- Botao CUE (30 x 14 px) ----
    cueButton.setBounds (xLocal (static_cast<int> (cux)), yLocal (static_cast<int> (cuy)),
                         static_cast<int> (cuw), static_cast<int> (cuh));

    // ---- Fader de volume (trilho 2 x 46, botao 9.94 x 7) ----
    // O componente contem a uniao do trilho e do botao (para nada ser cortado
    // pelo clip do JUCE), com o trilho centralizado horizontalmente de modo
    // que centroTrilhoLocal == width/2 do componente.
    const auto btnW = djdeck::layout::figmaNodemixerVolBtn1::widthPx;      // 9.94
    const auto compW = juce::roundToInt (btnW);                             // 10
    const auto railCentreAbs = vfx + vfw / 2.0;                             // centro do trilho em Figma abs
    const auto compLeftX = juce::roundToInt (railCentreAbs - mixerAbsX - channelOffsetX - compW / 2.0);

    volumeFader.setBounds (compLeftX, yLocal (static_cast<int> (vfy)), compW, static_cast<int> (vfh));
}

void ChannelStripComponent::sliderValueChanged (juce::Slider* slider)
{
    if (player == nullptr)
        return;

    if (slider == &volumeFader)
        player->setGain (volumeFader.getValue());
}

