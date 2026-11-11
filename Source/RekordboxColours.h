#pragma once

#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Paleta de cores do djdeck.
//
// Os valores foram medidos por amostragem de pixel das capturas do Rekordbox
// usadas como referencia — ver docs/DOCUMENTACAO_LAYOUT_UI.md §1.
//
// Os nomes são SEMANTICOS, não descritivos: trocar a paleta não deve tocar no
// codigo de desenho. Nenhum componente deve escrever um literal de cor.
//
// A paleta vive num header-only namespace de constantes inline: nao ha estado
// global mutavel, e o compilador descarta tudo que nao for usado.
namespace RekordboxColours
{
    // ---- Superficies -------------------------------------------------------
    inline const juce::Colour background      { juce::Colour (0xff191919) }; // #191919
    inline const juce::Colour panel           { juce::Colour (0xff1a1a1a) }; // #1A1A1A
    inline const juce::Colour panelRaised     { juce::Colour (0xff282828) }; // #282828
    inline const juce::Colour mixerBackground { juce::Colour (0xff121212) }; // #121212
    inline const juce::Colour canvas          { juce::Colour (0xff000000) }; // #000000
    inline const juce::Colour separator       { juce::Colour (0xff323232) }; // #323232

    // ---- Texto -------------------------------------------------------------
    inline const juce::Colour textPrimary     { juce::Colour (0xffffffff) }; // #FFFFFF
    inline const juce::Colour textSecondary   { juce::Colour (0xffcccccc) }; // #CCCCCC
    inline const juce::Colour textTertiary    { juce::Colour (0xffaeaeae) }; // #AEAEAE
    inline const juce::Colour textDisabled    { juce::Colour (0xff707070) }; // #707070

    // ---- Estados -----------------------------------------------------------
    inline const juce::Colour accent          { juce::Colour (0xff007de1) }; // #007DE1
    inline const juce::Colour selection       { juce::Colour (0xff052e64) }; // #052E64
    inline const juce::Colour selectionText   { juce::Colour (0xff158016) }; // #158016
    inline const juce::Colour hover           { juce::Colour (0xff2a2a2a) }; // #2A2A2A

    // ---- Semanticas de DJ --------------------------------------------------
    inline const juce::Colour loop            { juce::Colour (0xffff8c00) }; // #FF8C00
    inline const juce::Colour cue             { juce::Colour (0xffa63031) }; // #A63031
    inline const juce::Colour rating          { juce::Colour (0xffdeab38) }; // #DEAB38
    inline const juce::Colour beatGrid        { juce::Colour (0xffb8b8b8) }; // beat fino
    inline const juce::Colour downbeat        { juce::Colour (0xffa30000) }; // beat grosso
    inline const juce::Colour needle          { juce::Colour (0xffffffff) }; // agulha

    // ---- Waveform ----------------------------------------------------------
    inline const juce::Colour waveLow         { juce::Colour (0xff006eff) }; // #006EFF
    inline const juce::Colour waveMid         { juce::Colour (0xff00b4c2) }; // #00B4C2
    inline const juce::Colour waveHigh        { juce::Colour (0xff3eb4b4) }; // #3EB4B4
    inline const juce::Colour waveRegion      { juce::Colour (0xff805200) }; // #805200

    // ---- Hot cue pads — 8 cores distintas ----------------------------------
    inline const juce::Colour hotCue[8] =
    {
        juce::Colour (0xff007aff), // A azul
        juce::Colour (0xff00ff00), // B verde
        juce::Colour (0xffaf7ac7), // C roxo
        juce::Colour (0xffff8c00), // D laranja
        juce::Colour (0xff00b4c2), // E teal
        juce::Colour (0xffff6ec7), // F rosa
        juce::Colour (0xffffd700), // G amarelo
        juce::Colour (0xffff0000)  // H vermelho
    };
}
