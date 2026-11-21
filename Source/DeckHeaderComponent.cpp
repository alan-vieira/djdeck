#include "DeckHeaderComponent.h"
#include "LayoutConfig.hpp"
#include <cmath>

//==============================================================================
DeckHeaderComponent::DeckHeaderComponent (int deckIdToUse, bool isDeck2ToUse)
    : deckId (deckIdToUse),
      isDeck2 (isDeck2ToUse)
{
    syncButton.setClickingTogglesState (true);
    syncButton.setTooltip ("Sincroniza este deck com o deck master");
    syncButton.addListener (this);
    addAndMakeVisible (syncButton);

    masterButton.setClickingTogglesState (true);
    masterButton.setTooltip ("Define este deck como master");
    masterButton.setColour (juce::TextButton::buttonOnColourId, djdeck::layout::colours::Loop);
    masterButton.addListener (this);
    addAndMakeVisible (masterButton);
}

void DeckHeaderComponent::setDeckPosition (int x, int y)
{
    deckAbsPosition = juce::Point<int> (x, y);
}

DeckHeaderComponent::~DeckHeaderComponent() = default;

//==============================================================================
void DeckHeaderComponent::setTrackTitle (const juce::String& title)
{
    trackTitle = title.isNotEmpty() ? title : juce::String ("Sem faixa");
    repaint();
}

void DeckHeaderComponent::setTrackArtist (const juce::String& artist)
{
    trackArtist = artist;
    repaint();
}

void DeckHeaderComponent::setBpm (double newBpm)
{
    juce::Logger::writeToLog ("[DIAG] DeckHeaderComponent::setBpm called with " + juce::String (newBpm, 2));
    bpm = newBpm;
    repaint();
}

void DeckHeaderComponent::setKey (const juce::String& key)
{
    camelotKey.setMusicalKey (key);
    repaint();
}

void DeckHeaderComponent::setRawKey (const juce::String& key)
{
    trackKey = key;
    repaint();
}

void DeckHeaderComponent::setTimes (double elapsed, double remaining, double duration)
{
    elapsedSeconds = elapsed;
    remainingSeconds = remaining;
    durationSeconds = duration;
    repaint();
}

//==============================================================================
void DeckHeaderComponent::buttonClicked (juce::Button* button)
{
    if (button == &syncButton)
    {
        if (onSyncToggled != nullptr)
            onSyncToggled (syncButton.getToggleState());
    }
    else if (button == &masterButton)
    {
        if (onMasterToggled != nullptr)
            onMasterToggled (masterButton.getToggleState());
    }
}

//==============================================================================
void DeckHeaderComponent::drawDeckBadge (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour (djdeck::layout::colours::Primary);
    g.setFont (juce::Font (juce::FontOptions ("Inter", 14.0f, juce::Font::bold)));
    g.drawText (juce::String (deckId), area, juce::Justification::centredLeft, true);
}

//==============================================================================
void DeckHeaderComponent::drawArtworkPlaceholder (juce::Graphics& g,
                                                   const juce::Rectangle<int>& area)
{
    if (area.isEmpty())
        return;

    const auto bounds = area.toFloat();
    g.setColour (djdeck::layout::colours::Artwork);
    g.fillRoundedRectangle (bounds, 2.0f);

    g.setColour (juce::Colour (0xff042433));
    g.drawRoundedRectangle (bounds, 2.0f, 2.0f);

    // Disco no centro: contorno de um vinil com furo central.
    const auto centre = bounds.getCentre();
    constexpr float radius = 11.0f;
    g.setColour (juce::Colour (0xff042433));
    g.drawEllipse (centre.x - radius, centre.y - radius,
                   radius * 2.0f, radius * 2.0f, 2.0f);
    g.fillEllipse (centre.x - 1.5f, centre.y - 1.5f, 3.0f, 3.0f);
}

//==============================================================================
void DeckHeaderComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (djdeck::layout::colours::Panel);

    g.setColour (djdeck::layout::colours::Separator);
    g.drawLine (0.0f, static_cast<float> (area.getBottom() - 1),
                static_cast<float> (area.getRight()), static_cast<float> (area.getBottom() - 1), 1.0f);

    drawDeckBadge (g, deckNumberArea);
    drawArtworkPlaceholder (g, artworkArea);

    // BPM (70% da largura do nó)
    g.setFont (juce::Font (juce::FontOptions ("Inter", 11.0f, juce::Font::plain)));
    g.setColour (djdeck::layout::colours::Primary);
    g.drawText (bpm > 0.0 ? juce::String (bpm, 2) : juce::String ("--.--"), bpmArea, juce::Justification::centredLeft, false);

    // Tom (30% da largura do nó) - COLUNA SEPARADA
    g.setFont (juce::Font (juce::FontOptions ("Inter", 11.0f, juce::Font::plain)));
    g.setColour (djdeck::layout::colours::Secondary);
    g.drawText (trackKey.isEmpty() ? "--" : trackKey, tomArea, juce::Justification::centred, false);

    // Titulo e artista
    g.setFont (juce::Font (juce::FontOptions ("Inter", 15.0f, juce::Font::bold)));
    g.setColour (djdeck::layout::colours::Primary);
    g.drawText (trackTitle, titleArea, juce::Justification::centredLeft, false);

    g.setFont (juce::Font (juce::FontOptions ("Inter", 11.0f, juce::Font::plain)));
    g.setColour (djdeck::layout::colours::Secondary);
    g.drawText (trackArtist, artistArea, juce::Justification::centredLeft, false);

    // Tempos: regressiva e decorrido
    const auto formatTime = [] (double seconds)
    {
        const auto sign = seconds < 0.0 ? juce::String ("-") : juce::String();
        const auto abs = std::abs (seconds);

        const auto totalTenths = juce::roundToInt (abs * 10.0);
        const auto minutes = totalTenths / 600;
        const auto remainingTenths = totalTenths % 600;
        const auto secs = remainingTenths / 10;
        const auto tenths = remainingTenths % 10;

        return sign + juce::String (minutes).paddedLeft ('0', 2) + ":"
                     + juce::String (secs).paddedLeft ('0', 2) + "."
                     + juce::String (tenths);
    };

    g.setFont (juce::Font (juce::FontOptions ("Inter", 14.0f, juce::Font::bold)));
    g.setColour (djdeck::layout::colours::Secondary);

    auto timerTextArea = timerArea;

    // Contagem regressiva
    const auto remainingColour = remainingSeconds <= 10.0 ? djdeck::layout::colours::Cue
                                                            : djdeck::layout::colours::Secondary;
    g.setColour (remainingColour);
    g.drawText (formatTime (-remainingSeconds),
                timerTextArea.removeFromLeft (djdeck::layout::deckHeader::ElapsedtimeWidthPx),
                juce::Justification::centredLeft,
                false);

    g.setColour (djdeck::layout::colours::Secondary);
    g.drawText (formatTime (elapsedSeconds),
                timerTextArea.removeFromLeft (djdeck::layout::deckHeader::RemainingtimeWidthPx),
                juce::Justification::centredLeft,
                false);
}

//==============================================================================
void DeckHeaderComponent::resized()
{
    auto area = getLocalBounds();

    // Coordenadas relativas do Figma (deck 1)
    const auto relX = -deckAbsPosition.getX();
    const auto relY = -deckAbsPosition.getY();

    if (isDeck2)
    {
        deckNumberArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck2Number::xPx + relX,
            djdeck::layout::figmaNodedeck2Number::yPx + relY,
            djdeck::layout::figmaNodedeck2Number::widthPx,
            djdeck::layout::figmaNodedeck2Number::heightPx);

        artworkArea = juce::Rectangle<int> (
            djdeck::layout::deckHeader::ArtworkXpx,
            djdeck::layout::deckHeader::ArtworkYpx,
            djdeck::layout::deckHeader::ArtworkWidthPx,
            djdeck::layout::deckHeader::ArtworkHeightPx);

        titleArea = juce::Rectangle<int> (
            71,
            djdeck::layout::figmaNodedeck2Title::yPx + relY,
            djdeck::layout::deckHeader::TitleWidthPx,
            djdeck::layout::deckHeader::TitleHeightPx);

        artistArea = juce::Rectangle<int> (
            71,
            djdeck::layout::figmaNodedeck2Artist::yPx + relY,
            djdeck::layout::deckHeader::ArtistWidthPx,
            djdeck::layout::deckHeader::ArtistHeightPx);

        bpmArea = juce::Rectangle<int> (
            236,
            djdeck::layout::figmaNodedeck2BpmKey::yPx + relY,
            djdeck::layout::figmaNodedeck2BpmKey::widthPx,
            djdeck::layout::deckHeader::BpmkeyHeightPx);

        // NEW: TomArea é o restante do nó BPMKey (30% da largura)
        tomArea = juce::Rectangle<int> (
            bpmArea.getX() + ((bpmArea.getWidth() * 70) / 100),
            bpmArea.getY(),
            (bpmArea.getWidth() * 30) / 100,
            bpmArea.getHeight());

        timerArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck2Timer::xPx + relX,
            djdeck::layout::figmaNodedeck2Timer::yPx + relY,
            djdeck::layout::figmaNodedeck2Timer::widthPx,
            djdeck::layout::figmaNodedeck2Timer::heightPx);
    }
    else
    {
        deckNumberArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck1Number::xPx + relX,
            djdeck::layout::figmaNodedeck1Number::yPx + relY,
            djdeck::layout::figmaNodedeck1Number::widthPx,
            djdeck::layout::figmaNodedeck1Number::heightPx);

        artworkArea = juce::Rectangle<int> (
            djdeck::layout::deckHeader::ArtworkXpx,
            djdeck::layout::deckHeader::ArtworkYpx,
            djdeck::layout::deckHeader::ArtworkWidthPx,
            djdeck::layout::deckHeader::ArtworkHeightPx);

        titleArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck1Title::xPx + relX,
            djdeck::layout::figmaNodedeck1Title::yPx + relY,
            djdeck::layout::figmaNodedeck1Title::widthPx,
            djdeck::layout::figmaNodedeck1Title::heightPx);

        artistArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck1Artist::xPx + relX,
            djdeck::layout::figmaNodedeck1Artist::yPx + relY,
            djdeck::layout::figmaNodedeck1Artist::widthPx,
            djdeck::layout::figmaNodedeck1Artist::heightPx);

        bpmArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck1BpmKey::xPx + relX,
            djdeck::layout::figmaNodedeck1BpmKey::yPx + relY,
            djdeck::layout::figmaNodedeck1BpmKey::widthPx,
            djdeck::layout::figmaNodedeck1BpmKey::heightPx);

        // NEW: TomArea é o restante do nó BPMKey (30% da largura)
        tomArea = juce::Rectangle<int> (
            bpmArea.getX() + ((bpmArea.getWidth() * 70) / 100),
            bpmArea.getY(),
            (bpmArea.getWidth() * 30) / 100,
            bpmArea.getHeight());

        timerArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck1Timer::xPx + relX,
            djdeck::layout::figmaNodedeck1Timer::yPx + relY,
            djdeck::layout::figmaNodedeck1Timer::widthPx,
            djdeck::layout::figmaNodedeck1Timer::heightPx);
    }

    // ---- Botoes SYNC e MASTER, empilhados no canto direito ----
    // SYNC fica acima e MASTER abaixo. A pilha usa apenas uma coluna,
    // liberando espaco horizontal para o titulo da faixa.
    constexpr int rightMargin = 4;
    constexpr int verticalGap = 2;
    constexpr int topMargin = 1;

    const auto buttonWidth = djdeck::layout::deckHeader::SyncWidthPx;
    const auto buttonHeight = djdeck::layout::deckHeader::SyncHeightPx;
    const auto buttonRight = area.getRight() - rightMargin;

    syncButton.setBounds (buttonRight - buttonWidth,
                          topMargin,
                          buttonWidth,
                          buttonHeight);
    masterButton.setBounds (buttonRight - djdeck::layout::deckHeader::MasterWidthPx,
                             topMargin + buttonHeight + verticalGap,
                             djdeck::layout::deckHeader::MasterWidthPx,
                             djdeck::layout::deckHeader::MasterHeightPx);
}