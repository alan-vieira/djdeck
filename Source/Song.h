#pragma once

#include <juce_core/juce_core.h>
#include "TrackMetadata.h"

// Modelo simples de faixa da playlist
struct Song
{
    juce::File file;
    juce::String title;
    juce::String artist;
    double lengthInSeconds = 0.0;

    // Metadados lidos pelo TagLib (docs/ROADMAP.md — "Biblioteca para
    // metadados"). Preenchidos quando a faixa e adicionada a biblioteca.
    juce::String album;
    juce::String genre;
    juce::String key;          // notacao classica: "C#m", "Eb", ...
    juce::String year;
    juce::String trackNumber;
    double bpm = 0.0;          // 0 = desconhecido
    int rating = 0;            // 0..5 estrelas

    Song() = default;
    explicit Song(const juce::File& f);

    // Aplica os metadados lidos do arquivo.
    void applyMetadata (const TrackMetadata& metadata);

    juce::String getDisplayName() const;

    // Duracao formatada, ex.: "3:45".
    juce::String getFormattedLength() const;
};
