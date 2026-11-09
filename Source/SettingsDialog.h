#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

// Segunda janela com o seletor de dispositivo de áudio do JUCE.
// Permite escolher placa de som, sample rate e buffer size.
//
// IMPORTANTE: o seletor é alocado no heap e entregue ao DialogWindow via
// setContentOwned(). O DialogWindow passa a ser o dono e o deleta no
// fechamento. Declarar o seletor como member por valor e passá-lo ao
// setContentOwned() causaria double-delete (o member também seria destruído).
class SettingsDialog : public juce::DialogWindow
{
public:
    explicit SettingsDialog (juce::AudioDeviceManager& deviceManagerToUse)
        : DialogWindow ("Configurações de Áudio",
                        juce::Desktop::getInstance().getDefaultLookAndFeel()
                            .findColour (juce::ResizableWindow::backgroundColourId),
                        true)   // escapeKeyTriggersCloseButton
    {
        setUsingNativeTitleBar (true);

        setContentOwned (new juce::AudioDeviceSelectorComponent (deviceManagerToUse,
                                                                 0, 256,   // min/max input channels
                                                                 0, 256,   // min/max output channels
                                                                 true,     // show MIDI input options
                                                                 true,     // show MIDI output options
                                                                 true,     // show channels as stereo pairs
                                                                 false),   // don't hide advanced options
                       true);      // resize window when content changes size

        setResizable (true, false);
        setSize (600, 500);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    ~SettingsDialog() override = default;

    void closeButtonPressed() override
    {
        setVisible (false);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsDialog)
};
