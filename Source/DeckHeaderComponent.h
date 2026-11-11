#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "CamelotKeyComponent.h"

//==============================================================================
// Cabecalho do deck: informacoes da faixa carregada.
//
//   ┌────────────────────────────────────────────────────────────┐
//   │ (1)  Título da Faixa                   ╭───╮   [SYNC][MASTER]
//   │      Artista                           │ 3A│                │
//   │                            -05:10.5   ╰───╯   00:03.2    │
//   └────────────────────────────────────────────────────────────┘
//
// | Elemento            | Posicao      | Estilo                       |
// |---------------------|--------------|------------------------------|
// | Numero do deck      | topo esquerdo| circulo, 18 px, branco       |
// | Titulo              | apos numero  | 16 px, branco                |
// | Artista             | abaixo       | 13 px, cinza                 |
// | BPM                 | com a key    | 16 px, branco                |
// | Tom (Camelot wheel) | com o BPM    | roda de 12 segmentos         |
// | Tempo decorrido     | rodape       | 11 px, cinza                 |
// | Contagem regressiva | rodape       | 11 px, cinza                 |
// | Duracao da faixa    | rodape       | 11 px, cinza                 |
// | SYNC                | topo direito | botao; azul quando ativo     |
// | MASTER              | ao lado      | laranja quando ativo         |
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §7.1.
class DeckHeaderComponent : public juce::Component,
                            public juce::Button::Listener
{
public:
    DeckHeaderComponent (int deckIdToUse);
    ~DeckHeaderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Button::Listener
    void buttonClicked (juce::Button* button) override;

    // Metadados da faixa. Titulo, artista, BPM e key ainda nao vem do
    // TagLib — a deteccao de BPM e a leitura de metadados estao pendentes
    // (ver docs/ROADMAP.md). Enquanto isso, os valores sao placeholders.
    void setTrackTitle (const juce::String& title);
    void setTrackArtist (const juce::String& artist);
    void setBpm (double bpm);

    // Tom musical. Aceita a notacao classica ("C#m", "Eb") e converte para
    // Camelot, que e o formato deHarmonia usado em software de DJ.
    void setKey (const juce::String& key);

    // Tempos, em segundos:
    //   elapsed   — tempo decorrido desde o inicio
    //   remaining — contagem regressiva ate o fim da faixa
    //   duration  — duracao total da faixa
    void setTimes (double elapsed, double remaining, double duration);

    // Estado dos botoes.
    bool isSyncActive() const { return syncButton.getToggleState(); }
    bool isMasterActive() const { return masterButton.getToggleState(); }

    // Chamado quando o usuario alterna SYNC ou MASTER.
    std::function<void (bool isSync)> onSyncToggled;
    std::function<void (bool isMaster)> onMasterToggled;

private:
    void drawDeckBadge (juce::Graphics& g, const juce::Rectangle<int>& area);

    int deckId;

    juce::String trackTitle { "Sem faixa" };
    juce::String trackArtist;
    double bpm = 0.0;

    double elapsedSeconds = 0.0;
    double remainingSeconds = 0.0;
    double durationSeconds = 0.0;

    juce::TextButton syncButton { "SYNC" };
    juce::TextButton masterButton { "MASTER" };

    // Roda do tom musical (Camelot).
    CamelotKeyComponent camelotKey;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckHeaderComponent)
};
