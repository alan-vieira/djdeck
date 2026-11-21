#include "Song.h"
#include <cmath>

Song::Song(const juce::File& f)
    : file(f),
      title(f.getFileNameWithoutExtension()),
      artist("Artista desconhecido"),
      lengthInSeconds(0.0)
{}

juce::String Song::getDisplayName() const
{
    if (!artist.isEmpty() && artist != "Artista desconhecido")
        return artist + " - " + title;
    return title;
}

//==============================================================================
void Song::applyMetadata (const TrackMetadata& metadata)
{
    if (metadata.title.isNotEmpty())
        title = metadata.title;

    if (metadata.artist.isNotEmpty())
        artist = metadata.artist;

    album = metadata.album;
    genre = metadata.genre;
    key = metadata.key; // FIXED: Use public member
    year = metadata.year;
    trackNumber = metadata.trackNumber;
    bpm = metadata.bpm;
    rating = metadata.rating;

    if (metadata.durationSeconds > 0.0)
        lengthInSeconds = metadata.durationSeconds;
}

//==============================================================================
juce::String Song::getFormattedLength() const
{
    const auto total = static_cast<int> (lengthInSeconds);
    const auto minutes = total / 60;
    const auto seconds = total % 60;

    return juce::String (minutes) + ":"
         + juce::String (seconds).paddedLeft ('0', 2);
}