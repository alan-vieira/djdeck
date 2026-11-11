#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Indicador do tom musical do deck.
//
// Mostra o tom como um BADGE (codigo Camelot + notacao musical classica),
// sem desenhar a roda.
//
//   ┌─────┐
//   │ 3A  │   <- codigo Camelot
//   │ C#m │   <- notacao classica
//   └─────┘
//
// A CLASSE E TAMBEM o conversor para Camelot: o formato de Harmonia usado em
// software de DJ. A conversao fica aqui porque e a base da identificacao
// tonal e do key lock que virao a ser implementados.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §7.1.
class CamelotKeyComponent : public juce::Component
{
public:
    CamelotKeyComponent();
    ~CamelotKeyComponent() override;

    void paint (juce::Graphics& g) override;

    // Define o tom pelo numero Camelot (1..6) e a letra ('A' ou 'B').
    // Numero fora da faixa = tom desconhecido.
    void setCamelotNumber (int number, char letter);

    // Define o tom pela notacao musical classica ("C", "C#m", "Eb", ...).
    // Converte para Camelot internamente. String vazia = desconhecido.
    void setMusicalKey (const juce::String& key);

    // Notacao Camelot atual, ex.: "3A". Vazio = desconhecido.
    juce::String getCamelotCode() const;

    // Notacao musical classica correspondente. Vazio = desconhecido.
    juce::String getMusicalKey() const;

    // Converte uma notacao musical classica para Camelot.
    // Retorna false se a chave nao for reconhecida.
    static bool musicalKeyToCamelot (const juce::String& key, int& numberOut, char& letterOut);

private:
    int camelotNumber = 0;    // 1..6; 0 = desconhecido
    char camelotLetter = ' '; // 'A' ou 'B'

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CamelotKeyComponent)
};
