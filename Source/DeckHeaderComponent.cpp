#include "DeckHeaderComponent.h"
#include "LayoutConfig.hpp"

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

    // Botao SYNC: texto e borda amarelos (conforme o HTML de referência).
    syncButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    syncButton.setColour (juce::TextButton::textColourOffId, djdeck::layout::colours::Accent);
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
    const auto centre = area.getCentre().toFloat();
    // Numero do deck: 7 x 17 px — circulo pequeno com o digito centrado.
    constexpr float radius = 3.5f;

    g.setColour (djdeck::layout::colours::Primary);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (djdeck::layout::colours::Background);
    g.setFont (juce::Font (juce::FontOptions ("Inter", 14.0f, juce::Font::bold)));
    g.drawText (juce::String (deckId), area, juce::Justification::centred, true);
}

//==============================================================================
void DeckHeaderComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (djdeck::layout::colours::Panel);

    // Borda inferior entre o cabecalho e o resto do deck.
    g.setColour (djdeck::layout::colours::Separator);
    g.drawLine (0.0f, static_cast<float> (area.getBottom() - 1),
                static_cast<float> (area.getRight()), static_cast<float> (area.getBottom() - 1), 1.0f);

    // ---- Circulo com o numero do deck ----
    drawDeckBadge (g, deckNumberArea);

    // BPM e badge do tom, a direita ----
    const auto bpmText = bpm > 0.0 ? juce::String (bpm, 2) : juce::String ("--.--");

    // BPM: 55 x 13 px (medida do Figma).
    g.setFont (juce::Font (juce::FontOptions ("Inter", 11.0f, juce::Font::plain)));
    g.setColour (djdeck::layout::colours::Primary);
    const auto bpmKeyHeight = isDeck2
        ? static_cast<int> (djdeck::layout::figmaNodedeck2BpmKey::heightPx)
        : static_cast<int> (djdeck::layout::figmaNodedeck1BpmKey::heightPx);
    g.drawText (bpmText, bpmArea.removeFromTop (bpmKeyHeight), juce::Justification::centredLeft, false);

    // ---- Titulo e artista ----
    g.setFont (juce::Font (juce::FontOptions ("Inter", 15.0f, juce::Font::bold)));
    g.setColour (djdeck::layout::colours::Primary);
    // Titulo: 161 x 18 px (medida do Figma).
    g.drawText (trackTitle, titleArea, juce::Justification::centredLeft, false);

    g.setFont (juce::Font (juce::FontOptions ("Inter", 11.0f, juce::Font::plain)));
    g.setColour (djdeck::layout::colours::Secondary);
    const auto artistHeight = isDeck2
        ? static_cast<int> (djdeck::layout::figmaNodedeck2Artist::heightPx)
        : static_cast<int> (djdeck::layout::figmaNodedeck1Artist::heightPx);
    g.drawText (trackArtist, artistArea.removeFromTop (artistHeight), juce::Justification::centredLeft, false);

    // ---- Tempos: decorrido, regressiva e duracao ----
    const auto formatTime = [] (double seconds)
    {
        const auto sign = seconds < 0.0 ? juce::String ("-") : juce::String();
        const auto abs = std::abs (seconds);

        const auto minutes = static_cast<int> (abs) / 60;
        const auto secs = abs - minutes * 60;

        // "-05:10.5" — largura estavel, para nao tremer.
        return sign + juce::String (minutes) + ":"
                      + juce::String (secs, 1).paddedLeft ('0', 4);
    };

    g.setFont (juce::Font (juce::FontOptions ("Inter", 14.0f, juce::Font::bold)));
    g.setColour (djdeck::layout::colours::Secondary);

    // Tempo decorrido / restante / duracao na area do timer (medida do Figma).
    const auto timerHeight = isDeck2
        ? static_cast<int> (djdeck::layout::figmaNodedeck2Timer::heightPx)
        : static_cast<int> (djdeck::layout::figmaNodedeck1Timer::heightPx);
    auto timerTextArea = timerArea.removeFromTop (timerHeight);
    g.drawText (formatTime (elapsedSeconds), timerTextArea.removeFromLeft (68), juce::Justification::centredLeft, false);

    // Contagem regressiva — destacada quando falta pouco.
    const auto remainingColour = remainingSeconds <= 10.0 ? djdeck::layout::colours::Cue
                                                            : djdeck::layout::colours::Secondary;
    g.setColour (remainingColour);
    g.drawText (formatTime (remainingSeconds), timerTextArea.removeFromLeft (djdeck::layout::deckHeader::RemainingtimeWidthPx), juce::Justification::centredLeft, false);

    // Duracao total da faixa
    g.setColour (djdeck::layout::colours::Tertiary);
    g.drawText (formatTime (durationSeconds), timerTextArea.removeFromLeft (djdeck::layout::deckHeader::RemainingtimeWidthPx), juce::Justification::centredLeft, false);
}

//==============================================================================
void DeckHeaderComponent::resized()
{
    auto area = getLocalBounds();

    // Posicoes dos elementos de texto: coordenadas absolutas do Figma menos
    // a posicao absoluta do deck (deckAbsPosition).
    // No Figma (deck 1, pai em x=0, y=139):
    //   numero:  x=6  y=139  w=7  h=17  -> relativo: x=6,  y=0
    //   titulo:  x=71 y=145  w=161 h=18 -> relativo: x=71, y=6
    //   artista: x=70 y=163  w=58  h=13 -> relativo: x=70, y=24
    //   bpm:     x=236 y=163 w=55  h=13 -> relativo: x=236, y=24
    //   timer:   x=428 y=160 w=122 h=17 -> relativo: x=428, y=21
    const auto relX = -deckAbsPosition.getX();
    const auto relY = -deckAbsPosition.getY();

    if (isDeck2)
    {
        deckNumberArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck2Number::xPx + relX,
            djdeck::layout::figmaNodedeck2Number::yPx + relY,
            djdeck::layout::figmaNodedeck2Number::widthPx,
            djdeck::layout::figmaNodedeck2Number::heightPx);

        titleArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck2Title::xPx + relX,
            djdeck::layout::figmaNodedeck2Title::yPx + relY,
            djdeck::layout::figmaNodedeck2Title::widthPx,
            djdeck::layout::figmaNodedeck2Title::heightPx);

        artistArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck2Artist::xPx + relX,
            djdeck::layout::figmaNodedeck2Artist::yPx + relY,
            djdeck::layout::figmaNodedeck2Artist::widthPx,
            djdeck::layout::figmaNodedeck2Artist::heightPx);

        bpmArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck2BpmKey::xPx + relX,
            djdeck::layout::figmaNodedeck2BpmKey::yPx + relY,
            djdeck::layout::figmaNodedeck2BpmKey::widthPx,
            djdeck::layout::figmaNodedeck2BpmKey::heightPx);

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

        timerArea = juce::Rectangle<int> (
            djdeck::layout::figmaNodedeck1Timer::xPx + relX,
            djdeck::layout::figmaNodedeck1Timer::yPx + relY,
            djdeck::layout::figmaNodedeck1Timer::widthPx,
            djdeck::layout::figmaNodedeck1Timer::heightPx);
    }

    // Botoes SYNC e MASTER, no topo direito.
    const auto buttonHeight = juce::jmin (22, area.getHeight() / 2);

    auto masterArea = area.removeFromRight (64);
    auto syncArea = area.removeFromRight (djdeck::layout::deckHeader::SyncWidthPx + djdeck::layout::deckHeader::CamelotkeyWidthPx);

    // SYNC: 39 x 32 px, centralizado na sua area.
    auto syncBounds = syncArea.removeFromTop (djdeck::layout::deckHeader::SyncHeightPx);
    syncButton.setBounds (syncBounds.getX() + (syncBounds.getWidth() - djdeck::layout::deckHeader::SyncWidthPx) / 2,
                          syncBounds.getY(), djdeck::layout::deckHeader::SyncWidthPx, djdeck::layout::deckHeader::SyncHeightPx);

    masterButton.setBounds (masterArea.removeFromTop (buttonHeight).reduced (2));

    // O badge do tom fica logo abaixo do BPM, na coluna da direita.
    auto bpmKeyArea = area.removeFromRight (128);
    bpmKeyArea.removeFromTop (20);

    // Key (Camelot): 41 x 32 px, centralizado na sua area.
    auto keyBounds = bpmKeyArea.removeFromTop (djdeck::layout::deckHeader::CamelotkeyHeightPx);
    camelotKey.setBounds (keyBounds.getX() + (keyBounds.getWidth() - djdeck::layout::deckHeader::CamelotkeyWidthPx) / 2,
                          keyBounds.getY(), djdeck::layout::deckHeader::CamelotkeyWidthPx, djdeck::layout::deckHeader::CamelotkeyHeightPx);
}
