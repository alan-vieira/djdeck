#include <catch2/catch_test_macros.hpp>

#include "Song.h"

#include <juce_core/juce_core.h>

TEST_CASE("Song::getFormattedLength formata duracao em minutos:segundos", "[Song]")
{
    Song s(juce::File("test.mp3"));

    // Zero segundos
    s.lengthInSeconds = 0.0;
    REQUIRE(s.getFormattedLength() == "0:00");

    // 1 minuto e 5 segundos
    s.lengthInSeconds = 65.0;
    REQUIRE(s.getFormattedLength() == "1:05");

    // Uma hora e um segundo
    s.lengthInSeconds = 3661.0;
    REQUIRE(s.getFormattedLength() == "61:01");

    // Segundos com casa decimal sao truncados (int)
    s.lengthInSeconds = 59.9;
    REQUIRE(s.getFormattedLength() == "0:59");
}

TEST_CASE("Song::getDisplayName usa artista quando conhecido", "[Song]")
{
    Song s(juce::File("musica.mp3"));

    // Artista padrao desconhecido -> so o titulo
    s.title = "Minha Musica";
    s.artist = "Artista desconhecido";
    REQUIRE(s.getDisplayName() == "Minha Musica");

    // Artista vazio -> so o titulo
    s.artist = "";
    REQUIRE(s.getDisplayName() == "Minha Musica");

    // Artista real -> "Artista - Titulo"
    s.artist = "Daft Punk";
    REQUIRE(s.getDisplayName() == "Daft Punk - Minha Musica");
}

TEST_CASE("Song::applyMetadata preenche campos do TagLib", "[Song]")
{
    Song s(juce::File("arquivo.mp3"));

    TrackMetadata meta;
    meta.title = "Titulo Real";
    meta.artist = "Artista Real";
    meta.album = "Album Real";
    meta.genre = "House";
    meta.key = "8A";
    meta.year = "2024";
    meta.trackNumber = "3";
    meta.bpm = 124.0;
    meta.rating = 4;
    meta.durationSeconds = 240.0;
    meta.isValid = true;
    meta.hasTags = true;
    meta.yearNumber = 0;

    s.applyMetadata(meta);

    REQUIRE(s.title == "Titulo Real");
    REQUIRE(s.artist == "Artista Real");
    REQUIRE(s.album == "Album Real");
    REQUIRE(s.genre == "House");
    REQUIRE(s.key == "8A");
    REQUIRE(s.year == "2024");
    REQUIRE(s.trackNumber == "3");
    REQUIRE(s.bpm == 124.0);
    REQUIRE(s.rating == 4);
    REQUIRE(s.lengthInSeconds == 240.0);
}

TEST_CASE("Song::applyMetadata nao sobrescreve titulo/artista quando vazios", "[Song]")
{
    Song s(juce::File("arquivo.mp3"));
    s.title = "Titulo do Arquivo";
    s.artist = "Artista do Arquivo";

    TrackMetadata meta;
    meta.title = "";   // vazio -> nao sobrescreve
    meta.artist = "";  // vazio -> nao sobrescreve
    meta.album = "Novo Album";
    meta.bpm = 0.0;
    meta.durationSeconds = 0.0;
    meta.isValid = true;
    meta.hasTags = false;
    meta.yearNumber = 0;

    s.applyMetadata(meta);

    REQUIRE(s.title == "Titulo do Arquivo");
    REQUIRE(s.artist == "Artista do Arquivo");
    REQUIRE(s.album == "Novo Album");
}
