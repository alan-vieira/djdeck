#pragma once

#include <juce_core/juce_core.h>

// Modelo simples de faixa da playlist
struct Song
{
    juce::File file;
    juce::String title;
    juce::String artist;
    double lengthInSeconds = 0.0;

    Song() = default;
    explicit Song(const juce::File& f);

    juce::String getDisplayName() const;
};
