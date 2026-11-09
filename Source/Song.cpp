#include "Song.h"

Song::Song(const juce::File& f)
    : file(f),
      title(f.getFileNameWithoutExtension()),
      artist("Artista desconhecido"),
      lengthInSeconds(0.0)
{
}

juce::String Song::getDisplayName() const
{
    if (!artist.isEmpty() && artist != "Artista desconhecido")
        return artist + " - " + title;
    return title;
}
