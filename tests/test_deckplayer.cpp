#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "DeckPlayer.h"

using Catch::Approx;

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

static juce::AudioFormatManager& formatManager()
{
    static juce::AudioFormatManager fm;
    static bool registered = false;
    if (!registered)
    {
        fm.registerBasicFormats();
        registered = true;
    }
    return fm;
}

TEST_CASE("DeckPlayer::setGain / getGain - fader de canal", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    player.setGain(0.5);
    REQUIRE(player.getGain() == Approx(0.5));

    player.setGain(1.0);
    REQUIRE(player.getGain() == Approx(1.0));

    player.setGain(0.0);
    REQUIRE(player.getGain() == Approx(0.0));

    // Clamp: valores fora de 0..1 sao limitados
    player.setGain(-5.0);
    REQUIRE(player.getGain() == Approx(0.0));

    player.setGain(2.0);
    REQUIRE(player.getGain() == Approx(1.0));
}

TEST_CASE("DeckPlayer::setChannelGain - ganho de canal (usado pelo MIXING)", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    player.setChannelGain(0.3);
    REQUIRE(player.getGain() == Approx(0.3));

    player.setChannelGain(0.0);
    REQUIRE(player.getGain() == Approx(0.0));

    player.setChannelGain(1.0);
    REQUIRE(player.getGain() == Approx(1.0));
}

TEST_CASE("DeckPlayer::setTrimDb - gain de entrada em dB", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    player.setTrimDb(0.0);
    REQUIRE(player.getTrimDb() == Approx(0.0));

    player.setTrimDb(6.0);
    REQUIRE(player.getTrimDb() == Approx(6.0));

    player.setTrimDb(-6.0);
    REQUIRE(player.getTrimDb() == Approx(-6.0));

    // Clamp: +12 dB e -12 dB sao os limites
    player.setTrimDb(20.0);
    REQUIRE(player.getTrimDb() == Approx(12.0));

    player.setTrimDb(-20.0);
    REQUIRE(player.getTrimDb() == Approx(-12.0));
}

TEST_CASE("DeckPlayer::setCrossfadeGain / getCrossfadeGain", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    player.setCrossfadeGain(0.0);
    REQUIRE(player.getCrossfadeGain() == Approx(0.0));

    player.setCrossfadeGain(0.5);
    REQUIRE(player.getCrossfadeGain() == Approx(0.5));

    player.setCrossfadeGain(1.0);
    REQUIRE(player.getCrossfadeGain() == Approx(1.0));

    // Clamp
    player.setCrossfadeGain(-1.0);
    REQUIRE(player.getCrossfadeGain() == Approx(0.0));

    player.setCrossfadeGain(2.0);
    REQUIRE(player.getCrossfadeGain() == Approx(1.0));
}

TEST_CASE("DeckPlayer::setCue / isCue - estado PFL", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    REQUIRE_FALSE(player.isCue());

    player.setCue(true);
    REQUIRE(player.isCue());

    player.setCue(false);
    REQUIRE_FALSE(player.isCue());
}

TEST_CASE("DeckPlayer::setSpeed / getSpeed - variacao de pitch", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    player.setSpeed(1.0);
    REQUIRE(player.getSpeed() == Approx(1.0));

    player.setSpeed(0.5);  // limite inferior
    REQUIRE(player.getSpeed() == Approx(0.5));

    player.setSpeed(3.0);  // limite superior
    REQUIRE(player.getSpeed() == Approx(3.0));

    // Clamp abaixo de 0.5
    player.setSpeed(0.1);
    REQUIRE(player.getSpeed() == Approx(0.5));

    // Clamp acima de 3.0
    player.setSpeed(10.0);
    REQUIRE(player.getSpeed() == Approx(3.0));
}

TEST_CASE("DeckPlayer::setPositionRelative / getPositionRelative", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    // Sem arquivo carregado, comprimento e zero -> posicao relativa e zero
    REQUIRE(player.getPositionRelative() == Approx(0.0));

    // Definir posicao relativa nao falha sem arquivo
    player.setPositionRelative(0.5);
    // Como nao ha arquivo, getLengthInSeconds() e 0, e a posicao permanece 0
    REQUIRE(player.getPositionRelative() == Approx(0.0));
}

TEST_CASE("DeckPlayer::applyGain combina trim x channelGain x crossfadeGain", "[DeckPlayer]")
{
    DeckPlayer player(formatManager());

    // Valores neutros: trim 0 dB (=1.0 linear), channelGain 1.0, crossfadeGain 1.0
    player.setTrimDb(0.0);
    player.setChannelGain(1.0);
    player.setCrossfadeGain(1.0);
    // applyGain e chamado internamente pelos setters; so verificamos que nao trava
    // e que os getters refletem os valores definidos.
    REQUIRE(player.getTrimDb() == Approx(0.0));
    REQUIRE(player.getGain() == Approx(1.0));
    REQUIRE(player.getCrossfadeGain() == Approx(1.0));
}
