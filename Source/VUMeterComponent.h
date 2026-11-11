#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Medidor de nivel vertical, com LED de clip.
//
// Mede o sinal DEPOIS do fader de canal (pos-gain) — que e o que o DJ
// precisa ver: quanto esta saindo.
//
// O nivel cruza a fronteira entre a thread de audio e a thread de GUI. O
// DeckPlayer publica o valor ja em dB num juce::Atomic<float>; este
// componente apenas o le.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §5.7.
class VUMeterComponent : public juce::Component,
                         public juce::Timer
{
public:
    // `sourceLevelDb` e a referencia ao atomic publicado pelo DeckPlayer.
    // A referencia e mantida viva pelo DeckPlayer, que e destruido depois
    // dos componentes de UI.
    explicit VUMeterComponent (juce::Atomic<float>& sourceLevelDbToUse);
    ~VUMeterComponent() override;

    void paint (juce::Graphics& g) override;

    // Liga/desliga o corte automatico (clip). Desligado por padrao: o corte
    // depende de um segundo canal, que ainda nao existe.
    void setEnableClipDetection (bool shouldEnable);

private:
    void timerCallback() override;
    void drawScale (juce::Graphics& g, const juce::Rectangle<int>& area);

    juce::Atomic<float>& sourceLevelDb;
    float currentGain = 0.0f;
    float currentLevelDb = -100.0f;
    bool clipDetected = false;
    bool enableClipDetection = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMeterComponent)
};
