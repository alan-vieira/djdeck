#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

// Ouve mudanças no AudioDeviceManager (plug/unplug de dispositivos de áudio).
// Serve para detectar controladores USB e ajustar automaticamente o setup de
// áudio. Aqui, mantém-se atento e loga.
class MixerDeviceScanner : public juce::ChangeListener
{
public:
    explicit MixerDeviceScanner(juce::AudioDeviceManager& deviceManagerToUse)
        : deviceManager(deviceManagerToUse)
    {
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &deviceManager)
        {
            juce::Logger::writeToLog("MixerDeviceScanner: dispositivo de áudio alterado");
            // Aqui poderia-se escanear dispositivos conhecidos e reconfigurar
            // o deviceManager com sample rate / buffer size adequados.
        }
    }

private:
    juce::AudioDeviceManager& deviceManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerDeviceScanner)
};
