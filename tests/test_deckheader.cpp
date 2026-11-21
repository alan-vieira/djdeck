#include <catch2/catch_test_macros.hpp>
#include "DeckHeaderComponent.h"
#include "LayoutConfig.hpp"
#include <juce_graphics/juce_graphics.h>

#include <juce_core/juce_core.h>
#include <cmath>

namespace
{
    constexpr int deckWidth = 670;

    juce::Rectangle<int> relativeNode (int x, int y, int width, int height)
    {
        return { x - djdeck::layout::figmaNodedeck1::xPx,
                 y - djdeck::layout::figmaNodedeck1::yPx,
                 width,
                 height };
    }

    juce::Image renderHeader()
    {
        juce::Image image (juce::Image::ARGB,
                           deckWidth,
                           djdeck::layout::deckHeader::heightPx,
                           true);
        DeckHeaderComponent header (1, false);
        header.setDeckPosition (djdeck::layout::figmaNodedeck1::xPx,
                                djdeck::layout::figmaNodedeck1::yPx);
        header.setSize (deckWidth, djdeck::layout::deckHeader::heightPx);
        header.setTrackTitle ("Too Hot For Drumstep");
        header.setTrackArtist ("Wondawulf");
        header.setBpm (129.0);
        header.setTimes (0.4, 310.5, 310.9);
        header.setKey("D"); // Set key for the badge (will show classical D)
        header.setRawKey("D"); // Set raw key for Tom column
        juce::Graphics graphics (image);
        header.paint (graphics);
        return image;
    }

    // NEW: Test case for BPM pure + Tom column (70/30 split)
    juce::Image renderExpectedBpmTom()
    {
        juce::Image image (juce::Image::ARGB,
                           deckWidth,
                           djdeck::layout::deckHeader::heightPx,
                           true);
        image.clear (image.getBounds(), djdeck::layout::colours::Panel);

        juce::Graphics graphics (image);
        graphics.setFont (juce::Font (juce::FontOptions ("Inter", 11.0f, juce::Font::plain)));
        graphics.setColour (djdeck::layout::colours::Primary);
        
        // BPM (70% da largura do nó, centredLeft)
        graphics.drawText ("129.00",
                           relativeNode (djdeck::layout::figmaNodedeck1BpmKey::xPx,
                                         djdeck::layout::figmaNodedeck1BpmKey::yPx,
                                         djdeck::layout::figmaNodedeck1BpmKey::widthPx * 0.70,
                                         djdeck::layout::figmaNodedeck1BpmKey::heightPx),
                           juce::Justification::centredLeft,
                           false);
        
        // Tom (30% da largura do nó, centred)
        graphics.setColour (djdeck::layout::colours::Secondary);
        graphics.drawText ("D",
                           relativeNode (djdeck::layout::figmaNodedeck1BpmKey::xPx + (djdeck::layout::figmaNodedeck1BpmKey::widthPx * 0.70),
                                         djdeck::layout::figmaNodedeck1BpmKey::yPx,
                                         djdeck::layout::figmaNodedeck1BpmKey::widthPx * 0.30,
                                         djdeck::layout::figmaNodedeck1BpmKey::heightPx),
                           juce::Justification::centred,
                           false);
        
        return image;
    }

    bool samePixels (const juce::Image& actual,
                     const juce::Image& expected,
                     const juce::Rectangle<int>& area)
    {
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
                if (actual.getPixelAt (x, y) != expected.getPixelAt (x, y))
                    return false;
        return true;
    }

    bool containsVisibleContent (const juce::Image& image,
                                 const juce::Rectangle<int>& area,
                                 const juce::Colour& background)
    {
        for (int y = area.getY(); y < area.getBottom(); ++y)
            for (int x = area.getX(); x < area.getRight(); ++x)
                if (image.getPixelAt (x, y) != background)
                    return true;
        return false;
    }

    juce::Image renderDeck2Header()
    {
        juce::Image image (juce::Image::ARGB,
                           deckWidth,
                           djdeck::layout::deckHeader::heightPx,
                           true);
        DeckHeaderComponent header (2, true);
        header.setDeckPosition (djdeck::layout::figmaNodedeck2::xPx,
                                djdeck::layout::figmaNodedeck2::yPx);
        header.setSize (deckWidth, djdeck::layout::deckHeader::heightPx);
        header.setTrackTitle ("It's My Life (radio edit)");
        header.setTrackArtist ("Dr. Alban");
        header.setBpm (170.83);
        header.setTimes (4.8, 235.9, 240.8);
        header.setKey ("A");
        header.setRawKey ("8A");
        juce::Graphics graphics (image);
        header.paint (graphics);
        return image;
    }

    juce::TextButton* findButton (DeckHeaderComponent& header, const juce::String& text)
    {
        for (auto* child : header.getChildren())
            if (auto* button = dynamic_cast<juce::TextButton*> (child))
                if (button->getButtonText() == text)
                    return button;

        return nullptr;
    }
}

TEST_CASE ("Deck header paints BPM with pure BPM and classical Tom key in separate columns", "[DeckHeader]")
{
    const auto actual = renderHeader();
    const auto expected = renderExpectedBpmTom();

    const auto bpmTomArea = relativeNode (djdeck::layout::figmaNodedeck1BpmKey::xPx,
                                          djdeck::layout::figmaNodedeck1BpmKey::yPx,
                                          djdeck::layout::figmaNodedeck1BpmKey::widthPx,
                                          djdeck::layout::figmaNodedeck1BpmKey::heightPx);

    REQUIRE (samePixels (actual, expected, bpmTomArea));
}

TEST_CASE ("Deck header paints the artist in its Figma area", "[DeckHeader]")
{
    const auto actual = renderHeader();
    const auto artistArea = relativeNode (djdeck::layout::figmaNodedeck1Artist::xPx,
                                          djdeck::layout::figmaNodedeck1Artist::yPx,
                                          djdeck::layout::figmaNodedeck1Artist::widthPx,
                                          djdeck::layout::figmaNodedeck1Artist::heightPx);
    REQUIRE (containsVisibleContent (actual, artistArea, djdeck::layout::colours::Panel));
}

TEST_CASE ("Deck header paints negative remaining time and elapsed time", "[DeckHeader]")
{
    const auto actual = renderHeader();
    const auto timerArea = relativeNode (djdeck::layout::figmaNodedeck1Timer::xPx,
                                         djdeck::layout::figmaNodedeck1Timer::yPx,
                                         djdeck::layout::figmaNodedeck1Timer::widthPx,
                                         djdeck::layout::figmaNodedeck1Timer::heightPx);
    REQUIRE (containsVisibleContent (actual, timerArea, djdeck::layout::colours::Panel));
}

TEST_CASE ("Deck 2 title uses the full slot before BPM instead of the Figma sample width", "[DeckHeader]")
{
    const auto actual = renderDeck2Header();

    const auto titleX = djdeck::layout::figmaNodedeck2Title::xPx
                         - djdeck::layout::figmaNodedeck2::xPx;
    const auto titleY = djdeck::layout::figmaNodedeck2Title::yPx
                         - djdeck::layout::figmaNodedeck2::yPx;
    const auto bpmX = djdeck::layout::figmaNodedeck2BpmKey::xPx
                      - djdeck::layout::figmaNodedeck2::xPx;

    // O titulo deve continuar alem dos 49 px do texto de exemplo do Figma,
    // sem invadir a area do BPM (mantendo 4 px de separacao).
    const auto extendedTitleArea = juce::Rectangle<int> (
        titleX + djdeck::layout::figmaNodedeck2Title::widthPx,
        titleY,
        (bpmX - 4) - (titleX + djdeck::layout::figmaNodedeck2Title::widthPx),
        djdeck::layout::deckHeader::TitleHeightPx - 2);

    REQUIRE (containsVisibleContent (actual, extendedTitleArea,
                                     djdeck::layout::colours::Panel));
}

TEST_CASE ("Deck header shows an artwork indicator between deck number and title", "[DeckHeader]")
{
    const auto actual = renderDeck2Header();
    const auto expectedArtworkColour = juce::Colour (0xff156082);

    // O indicador de capa fica logo apos o numero do deck e antes do titulo.
    REQUIRE (actual.getPixelAt (31, 14) == expectedArtworkColour);
    REQUIRE (actual.getPixelAt (60, 30) == expectedArtworkColour);
}

TEST_CASE ("Deck header stacks SYNC above MASTER in the right corner", "[DeckHeader]")
{
    for (const auto deck2 : { false, true })
    {
        DeckHeaderComponent header (deck2 ? 2 : 1, deck2);
        header.setDeckPosition (deck2 ? djdeck::layout::figmaNodedeck2::xPx
                                      : djdeck::layout::figmaNodedeck1::xPx,
                              deck2 ? djdeck::layout::figmaNodedeck2::yPx
                                    : djdeck::layout::figmaNodedeck1::yPx);
        header.setSize (deckWidth, djdeck::layout::deckHeader::heightPx);

        auto* sync = findButton (header, "SYNC");
        auto* master = findButton (header, "MASTER");

        REQUIRE (sync != nullptr);
        REQUIRE (master != nullptr);
        REQUIRE (sync->isVisible());
        REQUIRE (master->isVisible());
        REQUIRE_FALSE (sync->getBounds().isEmpty());
        REQUIRE_FALSE (master->getBounds().isEmpty());

        // Ambos terminam no mesmo limite direito e formam uma pilha sem
        // sobreposicao: SYNC fica acima e MASTER abaixo.
        REQUIRE (sync->getBounds().getRight() == master->getBounds().getRight());
        REQUIRE (sync->getBounds().getBottom() <= master->getBounds().getY());
        REQUIRE (sync->getBounds().getY() < master->getBounds().getY());

        // A pilha fica no extremo direito do cabecalho, com 4 px de margem.
        REQUIRE (sync->getBounds().getRight() == header.getWidth() - 4);
        REQUIRE (master->getBounds().getRight() == header.getWidth() - 4);
    }
}
