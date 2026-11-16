//==============================================================================
//
//  LayoutConfig.hpp — GERADO AUTOMATICAMENTE a partir de Source/layout.json
//  NÃO EDITAR À MÃO. Rode: python tools/layout_codegen.py
//
//  Constantes de UI centralizadas para djdeck (cores, proporções,
//  tipografia, regras de espelhamento). Sem parsing de JSON em runtime.
//
//==============================================================================

#pragma once

#ifndef DJDECK_LAYOUT_CONFIG_HPP
#define DJDECK_LAYOUT_CONFIG_HPP

#include <juce_graphics/juce_graphics.h>

namespace djdeck {
namespace layout {

// Tela (px)
constexpr int screenWidth  = 1600;
constexpr int screenHeight = 900;

// Seções verticais (percentagem da altura da janela)
constexpr int sectionHeaderandfxHeightPercent = 10;
constexpr int sectionHeaderandfxHeightPx        = 90;
constexpr int sectionBigwaveformsHeightPercent = 22;
constexpr int sectionBigwaveformsHeightPx        = 200;
constexpr int sectionDecksandmixerHeightPercent = 39;
constexpr int sectionDecksandmixerHeightPx        = 350;
constexpr int sectionLibraryHeightPercent = 28;
constexpr int sectionLibraryHeightPx        = 250;

// Colunas da seção Decks+Mixer (% da largura)
constexpr int deckAWidthPercent  = 45;
constexpr int mixerWidthPercent  = 10;
constexpr int deckBWidthPercent  = 45;

// Colunas da Biblioteca (% da largura)
constexpr int libraryTreeWidthPercent  = 30;
constexpr int trackListWidthPercent    = 70;

// Espaçamento (px)
constexpr int gapTiny = 2;
constexpr int gapSmall = 4;
constexpr int gapMedium = 8;
constexpr int gapLarge = 16;
constexpr int panelInnerMargin = 8;
constexpr int sectionBorder = 1;

// Cores (valores medidos no Rekordbox — docs/DOCUMENTACAO_LAYOUT_UI.md §1)
namespace colours {
    // surfaces
    inline const juce::Colour Background = juce::Colour(0xff191919);
    inline const juce::Colour Panel = juce::Colour(0xff1A1A1A);
    inline const juce::Colour Panelraised = juce::Colour(0xff282828);
    inline const juce::Colour Mixerbackground = juce::Colour(0xff121212);
    inline const juce::Colour Canvas = juce::Colour(0xff000000);
    inline const juce::Colour Separator = juce::Colour(0xff323232);
    // text
    inline const juce::Colour Primary = juce::Colour(0xffFFFFFF);
    inline const juce::Colour Secondary = juce::Colour(0xffCCCCCC);
    inline const juce::Colour Tertiary = juce::Colour(0xffAEAEAE);
    inline const juce::Colour Disabled = juce::Colour(0xff707070);
    // states
    inline const juce::Colour Accent = juce::Colour(0xff007DE1);
    inline const juce::Colour Selection = juce::Colour(0xff052E64);
    inline const juce::Colour Selectiontext = juce::Colour(0xff158016);
    inline const juce::Colour Hover = juce::Colour(0xff2A2A2A);
    // djSemantics
    inline const juce::Colour Loop = juce::Colour(0xffFF8C00);
    inline const juce::Colour Cue = juce::Colour(0xffA63031);
    inline const juce::Colour Rating = juce::Colour(0xffDEAB38);
    inline const juce::Colour Beatgrid = juce::Colour(0xffB8B8B8);
    inline const juce::Colour Downbeat = juce::Colour(0xffA30000);
    inline const juce::Colour Needle = juce::Colour(0xffFFFFFF);
    // waveform
    inline const juce::Colour Low = juce::Colour(0xff006EFF);
    inline const juce::Colour Mid = juce::Colour(0xff00B4C2);
    inline const juce::Colour High = juce::Colour(0xff3EB4B4);
    inline const juce::Colour Region = juce::Colour(0xff805200);

    // Hot cue pads — 8 cores distintas
    inline const juce::Colour hotCue[8] = {
        juce::Colour(0xff007aff),
        juce::Colour(0xff00ff00),
        juce::Colour(0xffaf7ac7),
        juce::Colour(0xffff8c00),
        juce::Colour(0xff00b4c2),
        juce::Colour(0xffff6ec7),
        juce::Colour(0xffffd700),
        juce::Colour(0xffff0000)
    };
}

// Tipografia (px)
namespace typography {
    constexpr int displaySizePx = 28;
    constexpr int titleSizePx = 16;
    constexpr int bodySizePx = 13;
    constexpr int captionSizePx = 11;
    constexpr int microSizePx = 9;
}

// Regras de espelhamento dos decks
constexpr bool mirroringEnabled = true;

// Beat Jump / Hot Cue pads
constexpr bool PadsgridMirroredLeft  = true;
constexpr bool PadsgridMirroredRight = false;
// Jog wheel + CUE / Play
constexpr bool JogwheelMirroredLeft  = false;
constexpr bool JogwheelMirroredRight = true;
// CUE and play/pause
constexpr bool CueplaybuttonsMirroredLeft  = true;
constexpr bool CueplaybuttonsMirroredRight = false;
// EQ knob labels
constexpr bool EqlabelsMirroredLeft  = false;
constexpr bool EqlabelsMirroredRight = true;

// Especificações de componentes
namespace knob {
    constexpr float indicatorLengthPercent = 70;
    constexpr int   arcThicknessPx         = 2;
    constexpr float arcDegrees             = 270;
    constexpr int   labelSizePx            = 11;
    constexpr bool  labelBelow             = true;
}
namespace slider {
    namespace vertical {
        constexpr int railWidthPx    = 4;
        constexpr int handleHeightPx = 18;
    }
    namespace horizontal {
        constexpr int railHeightPx  = 4;
        constexpr int handleWidthPx = 24;
    }
}
namespace jogWheel {
    constexpr int diameterPx  = 140;
    constexpr int bpmSizePx   = 28;
    constexpr int pitchSizePx = 13;
}
namespace pads {
    constexpr int cols = 4;
    constexpr int rows = 2;
    constexpr int total = 8;
}
namespace vumeter {
    constexpr float minDb = -60;
    constexpr float maxDb = 0;
    constexpr int vumeterMarksDb[] = {
        0, -3, -6, -10, -16, -22, -32, -48,
    };
    constexpr int clipLedHeightPx = 4;
}
namespace eq {
    constexpr float minDb = -12;
    constexpr float maxDb = 12;
    constexpr float resolutionDb = 0.5;
    struct Band { const char* name; const char* type; float frequencyHz; float q; };
    constexpr Band bands[] = {
        {"LOW", "lowShelf", 250f, 0.7f},
        {"MID", "peak", 1000f, 1.0f},
        {"HIGH", "highShelf", 4000f, 0.7f},
    };
}
namespace crossfader {
    constexpr float centerGain = 0.707;
}
} // namespace layout
} // namespace djdeck

#endif // DJDECK_LAYOUT_CONFIG_HPP
