#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
// Metadados de uma faixa de audio.
//
// Populados pelo TagLib a partir do arquivo (ID3, Vorbis Comments, MP4, APE).
//
// A chave musical e armazenada na NOTACAO MUSICAL CLASSICA ("C#m", "Eb") e
// convertida para Camelot sob demanda — ver CamelotKeyComponent. O formato
// Camelot e o de Harmonia entre DJs.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §8.3.
struct TrackMetadata
{
    juce::String title;
    juce::String artist;
    juce::String album;
    juce::String genre;
    juce::String comment;
    juce::String key;          // notacao classica: "C#m", "Eb", ...
    juce::String year;
    juce::String trackNumber;

    double bpm = 0.0;          // 0 = desconhecido
    double durationSeconds = 0.0;
    int rating = 0;            // 0..5 estrelas
    int yearNumber = 0;

    // Verdadeiro se o arquivo tinha alguma tag.
    bool hasTags = false;

    // Verdadeiro se o arquivo era legivel pelo TagLib.
    bool isValid = false;

    juce::String getRatingStars() const;
};
