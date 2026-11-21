#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "CamelotKeyComponent.h"

//==============================================================================
// Cabecalho do deck: informacoes da faixa carregada.
//
// Top Row: Deck Number (18px icon), Title (16px), KEY SYNC (button)
// Bottom Row: Artist (13px), BPM (16px), Tom (16px), Remaining/Elapsed (11px)
//
// | Elemento            | Posicao      | Estilo                       |
// |---------------------|--------------|------------------------------|
// | Numero do deck      | topo esquerdo| circulo, 18 px, branco       |
// | Titulo              | apos numero  | 16 px, branco                |
// | Artista             | abaixo       | 13 px, cinza                 |
// | BPM                 | com a key    | 16 px, branco                |
// | Tom (Classical Key) | com o BPM    | 16 px, cinza                | <-- NEW: Tom em coluna separada
// | Tempo decorrido     | rodape       | 11 px, cinza                 |
// | Contagem regressiva | rodape       | 11 px, cinza                 |
// | Duracao da faixa    | rodape       | 11 px, cinza                 |
// | SYNC                | topo direito | botao; azul quando ativo     |
// | MASTER              | abaixo SYNC | laranja quando ativo         |
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §7.1.
class DeckHeaderComponent : public juce::Component,
                            public juce::Button::Listener
{
public:
    DeckHeaderComponent (int deckIdToUse, bool isDeck2ToUse = false);
    ~DeckHeaderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Button::Listener
    void buttonClicked (juce::Button* button) override;

    // Metadados da faixa.
    void setTrackTitle (const juce::String& title);
    void setTrackArtist (const juce::String& artist);
    void setBpm (double bpm);
    void setKey (const juce::String& key); // Set para o badge Camelot
    void setRawKey (const juce::String& key); // NEW: Set para a coluna Tom (classical notation)
    void setTimes (double elapsed, double remaining, double duration);

    // Estado dos botoes.
    bool isSyncActive() const { return syncButton.getToggleState(); }
    bool isMasterActive() const { return masterButton.getToggleState(); }

    // Chamado quando o usuario alterna SYNC ou MASTER.
    std::function<void (bool isSync)> onSyncToggled;
    std::function<void (bool isMaster)> onMasterToggled;

    // Define a posicao absoluta do deck no canvas do Figma (para posicionar
    // os elementos de texto nas coordenadas exatas dos nos figmaNode*).
    void setDeckPosition (int x, int y);

private:
    void drawDeckBadge (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawArtworkPlaceholder (juce::Graphics& g, const juce::Rectangle<int>& area);

    int deckId;
    bool isDeck2;
    juce::Point<int> deckAbsPosition { 0, 0 }; // posicao absoluta do deck no canvas do Figma

    juce::String trackTitle { "Sem faixa" };
    juce::String trackArtist;
    double bpm = 0.0;
    juce::String trackKey; // NEW: Armazena a chave musical (clássica ou Camelot)

    double elapsedSeconds = 0.0;
    double remainingSeconds = 0.0;
    double durationSeconds = 0.0;

    juce::Rectangle<int> deckNumberArea;
    juce::Rectangle<int> artworkArea;
    juce::Rectangle<int> titleArea;
    juce::Rectangle<int> artistArea;
    juce::Rectangle<int> bpmArea;
    juce::Rectangle<int> tomArea; // NEW: Area para a coluna Tom
    juce::Rectangle<int> timerArea;

    juce::TextButton syncButton { "SYNC" };
    juce::TextButton masterButton { "MASTER" };

    // Roda do tom musical (Camelot).
    CamelotKeyComponent camelotKey;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckHeaderComponent)
};