#include "DeckHeaderComponent.h"
#include "RekordboxColours.h"

//==============================================================================
DeckHeaderComponent::DeckHeaderComponent (int deckIdToUse)
    : deckId (deckIdToUse)
{
    syncButton.setClickingTogglesState (true);
    syncButton.setTooltip ("Sincroniza este deck com o deck master");
    syncButton.addListener (this);
    addAndMakeVisible (syncButton);

    masterButton.setClickingTogglesState (true);
    masterButton.setTooltip ("Define este deck como master");
    masterButton.setColour (juce::TextButton::buttonOnColourId, RekordboxColours::loop);
    masterButton.addListener (this);
    addAndMakeVisible (masterButton);
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
    constexpr float radius = 9.0f;

    g.setColour (RekordboxColours::textPrimary);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (RekordboxColours::background);
    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText (juce::String (deckId), area, juce::Justification::centred, true);
}

//==============================================================================
void DeckHeaderComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (RekordboxColours::panel);

    // Borda inferior entre o cabecalho e o resto do deck.
    g.setColour (RekordboxColours::separator);
    g.drawLine (0.0f, static_cast<float> (area.getBottom() - 1),
                static_cast<float> (area.getRight()), static_cast<float> (area.getBottom() - 1), 1.0f);

    // ---- Circulo com o numero do deck ----
    const auto badgeArea = area.removeFromLeft (26);
    drawDeckBadge (g, badgeArea.reduced (4, 4));

    // ---- BPM e badge do tom, a direita ----
    // O badge do tom e posicionado no resized(); aqui so desenhamos o BPM.
    auto bpmKeyArea = area.removeFromRight (128);

    const auto bpmText = bpm > 0.0 ? juce::String (bpm, 2) : juce::String ("--.--");

    g.setFont (juce::Font (juce::FontOptions (16.0f)));
    g.setColour (RekordboxColours::textPrimary);
    g.drawText (bpmText, bpmKeyArea.removeFromTop (20), juce::Justification::centredLeft, true);

    // ---- Titulo e artista ----
    auto textArea = area;

    g.setFont (juce::Font (juce::FontOptions (16.0f)));
    g.setColour (RekordboxColours::textPrimary);
    g.drawText (trackTitle, textArea.removeFromTop (20), juce::Justification::centredLeft, true);

    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.setColour (RekordboxColours::textSecondary);
    g.drawText (trackArtist, textArea.removeFromTop (18), juce::Justification::centredLeft, true);

    // ---- Tempos: decorrido, regressiva e duracao ----
    auto timeArea = area.removeFromBottom (14);

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

    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.setColour (RekordboxColours::textSecondary);

    // Tempo decorrido
    g.drawText (formatTime (elapsedSeconds), timeArea.removeFromLeft (68), juce::Justification::centredLeft, true);

    // Contagem regressiva — destacada quando falta pouco.
    const auto remainingColour = remainingSeconds <= 10.0 ? RekordboxColours::cue
                                                           : RekordboxColours::textSecondary;
    g.setColour (remainingColour);
    g.drawText (formatTime (remainingSeconds), timeArea.removeFromLeft (68), juce::Justification::centredLeft, true);

    // Duracao total da faixa
    g.setColour (RekordboxColours::textTertiary);
    g.drawText (formatTime (durationSeconds), timeArea.removeFromLeft (68), juce::Justification::centredLeft, true);
}

//==============================================================================
void DeckHeaderComponent::resized()
{
    auto area = getLocalBounds();

    // Botoes SYNC e MASTER, no topo direito.
    //
    // O layout fica aqui, e nao no paint(): chamar setBounds() durante o
    // desenho e reentrante e deixa o layout instavel.
    const auto buttonHeight = juce::jmin (22, area.getHeight() / 2);

    auto masterArea = area.removeFromRight (64);
    auto syncArea = area.removeFromRight (52);

    syncButton.setBounds (syncArea.removeFromTop (buttonHeight).reduced (2));
    masterButton.setBounds (masterArea.removeFromTop (buttonHeight).reduced (2));

    // O badge do tom fica logo abaixo do BPM, na coluna da direita.
    auto bpmKeyArea = area.removeFromRight (128);
    bpmKeyArea.removeFromTop (20);

    camelotKey.setBounds (bpmKeyArea.removeFromTop (30).reduced (2, 0));
}
