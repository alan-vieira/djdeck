#include "CamelotKeyComponent.h"
#include "RekordboxColours.h"

//==============================================================================
namespace
{
    // A roda Camelot: 12 posicoes em rodizio de quinta.
    //
    // Numero  Letra  Tonica menor   Tonica maior
    // ------  -----  -------------  -------------
    //   1       A        Ab             Bb
    //   1       B         B             Db
    //   2       A        Bb             Eb
    //   2       B         C             F
    //   3       A        C#             E
    //   3       B         D             G
    //   4       A        D#             F#
    //   4       B         E             A
    //   5       A         F             Ab
    //   5       B        F#             Bb
    //   6       A         G             C
    //   6       B        G#             D
    //
    // indices 0..11 no array = (numero - 1) * 2 + (letra == 'B' ? 1 : 0)
    struct CamelotEntry
    {
        const char* number;
        char letter;
        const char* minorTonic;
        const char* majorTonic;
    };

    const CamelotEntry camelotWheel[12] =
    {
        { "1", 'A', "Ab", "Bb" },
        { "1", 'B',  "B", "Db" },
        { "2", 'A', "Bb", "Eb" },
        { "2", 'B',  "C",  "F" },
        { "3", 'A', "C#",  "E" },
        { "3", 'B',  "D",  "G" },
        { "4", 'A', "D#", "F#" },
        { "4", 'B',  "E",  "A" },
        { "5", 'A',  "F", "Ab" },
        { "5", 'B', "F#", "Bb" },
        { "6", 'A',  "G",  "C" },
        { "6", 'B', "G#",  "D" }
    };

    // Normaliza o nome da tonica: aceita "Bb", "A#" e "Db" -> "B", "A", "C".
    // A ordem importa: os sustenos (naturais) vem ANTES dos bemóis.
    const char* const enharmonicNames[12] =
    {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
}

//==============================================================================
CamelotKeyComponent::CamelotKeyComponent()
{
    setInterceptsMouseClicks (false, false);
}

CamelotKeyComponent::~CamelotKeyComponent() = default;

//==============================================================================
void CamelotKeyComponent::setCamelotNumber (int number, char letter)
{
    if (number < 1 || number > 6)
    {
        camelotNumber = 0;
        camelotLetter = ' ';
        repaint();
        return;
    }

    camelotNumber = number;
    camelotLetter = (letter == 'B' || letter == 'b') ? 'B' : 'A';
    repaint();
}

//==============================================================================
void CamelotKeyComponent::setMusicalKey (const juce::String& key)
{
    int number = 0;
    char letter = ' ';

    if (key.isEmpty() || ! musicalKeyToCamelot (key, number, letter))
    {
        camelotNumber = 0;
        camelotLetter = ' ';
        repaint();
        return;
    }

    setCamelotNumber (number, letter);
}

//==============================================================================
juce::String CamelotKeyComponent::getCamelotCode() const
{
    if (camelotNumber < 1 || camelotNumber > 6)
        return {};

    return juce::String (camelotNumber) + juce::String::charToString (camelotLetter);
}

//==============================================================================
juce::String CamelotKeyComponent::getMusicalKey() const
{
    if (camelotNumber < 1 || camelotNumber > 6)
        return {};

    const auto index = (camelotNumber - 1) * 2 + (camelotLetter == 'B' ? 1 : 0);
    return juce::String (camelotWheel[index].minorTonic);
}

//==============================================================================
bool CamelotKeyComponent::musicalKeyToCamelot (const juce::String& key, int& numberOut, char& letterOut)
{
    numberOut = 0;
    letterOut = ' ';

    auto text = key.trim().toLowerCase();

    if (text.isEmpty())
        return false;

    // Remove o sufixo de modo: "m" para menor, "maj" para maior.
    bool isMinor = false;

    if (text.endsWith ("maj"))
    {
        text = text.substring (0, text.length() - 3);
    }
    else if (text.endsWith ("min") || text.endsWith ("m"))
    {
        text = text.substring (0, text.length() - (text.endsWith ("min") ? 3 : 1));
        isMinor = true;
    }

    text = text.trim();

    // Normaliza os bemóis/alternativos para sustenos.
    if (text == "bb") text = "a#";
    else if (text == "db") text = "c#";
    else if (text == "eb") text = "d#";
    else if (text == "gb") text = "f#";
    else if (text == "ab") text = "g#";
    else if (text == "cb") text = "b";

    // Encontra a tonica na tabela.
    int tonicIndex = -1;

    for (int i = 0; i < 12; ++i)
    {
        if (text == juce::String (enharmonicNames[i]).toLowerCase())
        {
            tonicIndex = i;
            break;
        }
    }

    if (tonicIndex < 0)
        return false;

    // Encontra a entrada da roda Camelot com essa tonica.
    for (int i = 0; i < 12; ++i)
    {
        const auto& entry = camelotWheel[i];
        const auto matches = isMinor ? (text == juce::String (entry.minorTonic).toLowerCase())
                                     : (text == juce::String (entry.majorTonic).toLowerCase());

        if (matches)
        {
            numberOut = entry.number[0] - '0';
            letterOut = entry.letter;
            return true;
        }
    }

    return false;
}

//==============================================================================
void CamelotKeyComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    const auto code = getCamelotCode();

    if (code.isEmpty())
    {
        // Tom desconhecido: badge vazio, sem destaque.
        g.setColour (RekordboxColours::panelRaised);
        g.fillRoundedRectangle (area, 2.0f);

        g.setColour (RekordboxColours::textDisabled);
        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.drawText ("--", area, juce::Justification::centred, true);
        return;
    }

    // Badge do tom: fundo vermelho, codigo Camelot em cima e notacao
    // classica embaixo.
    g.setColour (RekordboxColours::cue);
    g.fillRoundedRectangle (area, 2.0f);

    g.setColour (RekordboxColours::textPrimary);
    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.drawText (code, area.removeFromTop (area.getHeight() * 0.6f), juce::Justification::centred, true);

    g.setFont (juce::Font (juce::FontOptions (9.0f)));
    g.drawText (getMusicalKey(), area, juce::Justification::centred, true);
}
