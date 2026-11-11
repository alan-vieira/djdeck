#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Barra de topo global: modo, layout, modulos ativos e relogio.
//
// ┌────────────────────────────────────────────────────────────────────────┐
// │  djdeck   │ PERFORMANCE │ 2Deck Horizontal │  MIDI  Audio  │  14:32   │
// └────────────────────────────────────────────────────────────────────────┘
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §2.1 (header + barra FX, ~90 px / 10%).
class GlobalHeaderComponent : public juce::Component,
                              public juce::Timer
{
public:
    GlobalHeaderComponent();
    ~GlobalHeaderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Modo de exibicao (PERFORMANCE / PRACTICE / DJ VIEW).
    void setMode (const juce::String& mode);
    juce::String getMode() const { return mode; }

    // Layout de decks (2Deck Horizontal / 2Deck Vertical).
    void setLayoutName (const juce::String& layoutName);
    juce::String getLayoutName() const { return layoutName; }

private:
    void timerCallback() override;

    juce::String mode { "PERFORMANCE" };
    juce::String layoutName { "2Deck Horizontal" };

    // Modulos ativos, mostrados como pips coloridos.
    juce::StringArray activeModules { "MIDI", "AUDIO", "VIDEO" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalHeaderComponent)
};
