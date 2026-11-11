#include "ChannelStripComponent.h"
#include "RekordboxColours.h"

//==============================================================================
ChannelStripComponent::ChannelStripComponent (DeckPlayer* playerToUse, bool mirroredToUse)
    : player (playerToUse),
      mirrored (mirroredToUse),
      // `player` e declarado ANTES de `vuMeter` no header, e os members sao
      // inicializados na ordem da declaracao — entao o ponteiro ja vale aqui.
      vuMeter (playerToUse->getLevelSource (0))
{
    // ---- Knobs ----
    // TRIM: ±12 dB, neutro no centro (0 dB).
    trimKnob.setRange (-12.0f, 12.0f, 0.1f);
    trimKnob.setValue (0.0f);
    trimKnob.setDoubleClickReturnValue (true, 0.0);

    // EQ: ±12 dB, 0 dB = flat.
    highKnob.setRange (-12.0f, 12.0f, 0.5f);
    highKnob.setValue (0.0f);
    highKnob.setDoubleClickReturnValue (true, 0.0);

    midKnob.setRange (-12.0f, 12.0f, 0.5f);
    midKnob.setValue (0.0f);
    midKnob.setDoubleClickReturnValue (true, 0.0);

    lowKnob.setRange (-12.0f, 12.0f, 0.5f);
    lowKnob.setValue (0.0f);
    lowKnob.setDoubleClickReturnValue (true, 0.0);

    // Filtro: bipolar, 0 = neutro (bypass).
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
    addAndMakeVisible (cueButton);

    // ---- Slider de volume ----
    // Range LINEAR 0-1: mantem a API de DeckPlayer::setGain() sem conversao.
    volumeFader.setRange (0.0, 1.0);
    volumeFader.setValue (player != nullptr ? player->getGain() : 0.5);
    volumeFader.addListener (this);
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
    g.fillAll (RekordboxColours::mixerBackground);

    // Borda entre os canais.
    g.setColour (RekordboxColours::separator);
    g.drawRect (getLocalBounds(), 1);
}

//==============================================================================
void ChannelStripComponent::resized()
{
    auto area = getLocalBounds().reduced (4);

    // O VU fica numa coluna estreita AO LADO dos knobs.
    const auto vuWidth = juce::jlimit (14, 22, area.getWidth() / 6);
    auto vuArea = area.removeFromLeft (vuWidth);

    // O resto: coluna de knobs + coluna de botao/fader.
    auto controlsArea = area;

    // A coluna de botao/fader fica do lado oposto ao VU no deck espelhado.
    auto knobColumn = controlsArea;
    auto sideColumn = controlsArea;

    if (! mirrored)
        sideColumn = controlsArea.removeFromRight (juce::jmax (30, controlsArea.getWidth() / 4));
    else
        sideColumn = controlsArea.removeFromLeft (juce::jmax (30, controlsArea.getWidth() / 4));

    // ---- Botao CUE e fader na coluna lateral ----
    // O botao CUE tem tamanho fixo para nao ficar truncado.
    const auto cueSize = juce::jmin (24, sideColumn.getWidth() - 4);
    auto cueArea = sideColumn.removeFromTop (cueSize);
    cueButton.setBounds (cueArea.withSizeKeepingCentre (cueSize, cueSize));

    // O volume fader ocupa o resto da coluna lateral.
    volumeFader.setBounds (sideColumn.reduced (2));

    // ---- Knobs empilhados na coluna principal ----
    const auto knobCount = 5;
    const auto gap = 4;
    const auto knobHeight = juce::jmax (24, (knobColumn.getHeight() - gap * (knobCount - 1)) / knobCount);

    KnobComponent* knobs[] = { &trimKnob, &highKnob, &midKnob, &lowKnob, &filterKnob };

    for (auto* knob : knobs)
    {
        auto knobArea = knobColumn.removeFromTop (knobHeight);

        // O knob e quadrado: usa o menor lado, centralizado na faixa.
        const auto side = juce::jmin (knobArea.getWidth(), knobArea.getHeight());
        knob->setBounds (knobArea.withSizeKeepingCentre (side, side));

        if (! knobColumn.isEmpty())
            knobColumn.removeFromTop (gap);
    }

    vuMeter.setBounds (vuArea.reduced (1));
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
