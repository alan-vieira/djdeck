#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "CrossfaderComponent.h"

using Catch::Approx;

#include <cmath>

// computeGains e estatico, entao podemos chamar diretamente sem instanciar o componente.

TEST_CASE("CrossfaderComponent::computeGains - curva constant power", "[Crossfader]")
{
    float gainA = 0.0f;
    float gainB = 0.0f;

    // Toda a esquerda (position = -1.0): deck A no maximo, deck B silenciado
    CrossfaderComponent::computeGains(-1.0, gainA, gainB);
    REQUIRE(gainA == Approx(1.0f).margin(0.001f));
    REQUIRE(gainB == Approx(0.0f).margin(0.001f));

    // Centro (position = 0.0): mistura equilibrada, cada deck a -3 dB (0.707)
    CrossfaderComponent::computeGains(0.0, gainA, gainB);
    REQUIRE(gainA == Approx(0.7071f).margin(0.01f));
    REQUIRE(gainB == Approx(0.7071f).margin(0.01f));

    // Toda a direita (position = +1.0): deck B no maximo, deck A silenciado
    CrossfaderComponent::computeGains(1.0, gainA, gainB);
    REQUIRE(gainA == Approx(0.0f).margin(0.001f));
    REQUIRE(gainB == Approx(1.0f).margin(0.001f));
}

TEST_CASE("CrossfaderComponent::computeGains - potencia constante em qualquer posicao", "[Crossfader]")
{
    float gainA = 0.0f;
    float gainB = 0.0f;

    // Em qualquer posicao, gainA^2 + gainB^2 deve ser ~= 1.0 (potencia constante)
    for (double pos = -1.0; pos <= 1.0; pos += 0.125)
    {
        CrossfaderComponent::computeGains(pos, gainA, gainB);
        const auto power = gainA * gainA + gainB * gainB;
        REQUIRE(power == Approx(1.0f).margin(0.01f));
    }
}

TEST_CASE("CrossfaderComponent::computeGains - posicoes intermediarias", "[Crossfader]")
{
    float gainA = 0.0f;
    float gainB = 0.0f;

    // position = -0.5: mais deck A que deck B
    CrossfaderComponent::computeGains(-0.5, gainA, gainB);
    REQUIRE(gainA > gainB);
    REQUIRE(gainA == Approx(0.9239f).margin(0.01f));  // cos(3pi/8)
    REQUIRE(gainB == Approx(0.3827f).margin(0.01f));  // sin(3pi/8)

    // position = +0.5: mais deck B que deck A
    CrossfaderComponent::computeGains(0.5, gainA, gainB);
    REQUIRE(gainB > gainA);
    REQUIRE(gainA == Approx(0.3827f).margin(0.01f));
    REQUIRE(gainB == Approx(0.9239f).margin(0.01f));
}
