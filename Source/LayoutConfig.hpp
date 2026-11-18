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
constexpr int screenWidth  = 1452;
constexpr int screenHeight = 813;

// Seções verticais (percentagem da altura da janela)
constexpr int sectionBigwaveformsHeightPercent = 0;
constexpr int sectionBigwaveformsHeightPx        = 137;
constexpr int sectionDecksandmixerHeightPercent = 0;
constexpr int sectionDecksandmixerHeightPx        = 242;
constexpr int sectionLibraryHeightPercent = 0;
constexpr int sectionLibraryHeightPx        = 434;

// Colunas da seção Decks+Mixer (% da largura)
constexpr int deckAWidthPercent  = 45;
constexpr int mixerWidthPercent  = 10;
constexpr int deckBWidthPercent  = 45;

// Larguras absolutas dos decks (px, medida do Figma)
constexpr int deckAWidthPx  = 671;
constexpr int mixerWidthPx  = 109;
constexpr int deckBWidthPx  = 671;

// Alturas absolutas dos decks (px, medida do Figma)
constexpr int deckAHeightPx  = 242;
constexpr int mixerHeightPx  = 242;
constexpr int deckBHeightPx  = 242;

// Colunas da Biblioteca (% da largura)
constexpr int libraryTreeWidthPercent  = 249;
constexpr int trackListWidthPercent    = 1203;

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
    inline const juce::Colour Mixerbackground = juce::Colour(0xff0F0F14);
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
    inline const juce::Colour Hover = juce::Colour(0xff2A2A2A);
    // djSemantics
    inline const juce::Colour Loop = juce::Colour(0xffFF8C00);
    inline const juce::Colour Cue = juce::Colour(0xffA63031);
    inline const juce::Colour Rating = juce::Colour(0xffDEAB38);
    inline const juce::Colour Needle = juce::Colour(0xffFFFFFF);
    inline const juce::Colour Beatgrid = juce::Colour(0xffB8B8B8);
    inline const juce::Colour Downbeat = juce::Colour(0xffA30000);
    // waveform
    inline const juce::Colour Deck1 = juce::Colour(0xff007DE1);
    inline const juce::Colour Deck2 = juce::Colour(0xffA63031);
    inline const juce::Colour Low = juce::Colour(0xff006EFF);
    inline const juce::Colour Mid = juce::Colour(0xff00B4C2);
    inline const juce::Colour High = juce::Colour(0xff3EB4B4);
    inline const juce::Colour Region = juce::Colour(0xff805200);
    // mixer
    inline const juce::Colour KnobBody = juce::Colour(0xff2A2A2A);
    inline const juce::Colour KnobBorder = juce::Colour(0xff4D4D59);
    inline const juce::Colour VuMeter = juce::Colour(0xff26262E);
    inline const juce::Colour VolumeButton = juce::Colour(0xffCCCCD9);
    inline const juce::Colour VolumeRail = juce::Colour(0xff333340);
    inline const juce::Colour CueColor = juce::Colour(0xff2666F2);
    inline const juce::Colour TextPrimary = juce::Colour(0xffF2F2F2);

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
    inline const char* fontFamily = "Inter";
    constexpr int deckNumberSizePx = 14;
    constexpr int deckNumberWeight  = juce::Font::bold;
    constexpr int titleSizePx = 15;
    constexpr int titleWeight  = juce::Font::bold;
    constexpr int artistSizePx = 11;
    constexpr int artistWeight  = juce::Font::plain;
    constexpr int bpmSizePx = 11;
    constexpr int bpmWeight  = juce::Font::plain;
    constexpr int timeSizePx = 14;
    constexpr int timeWeight  = juce::Font::bold;
    constexpr int jogBpmSizePx = 15;
    constexpr int jogBpmWeight  = juce::Font::bold;
    constexpr int jogPitchSizePx = 9;
    constexpr int jogPitchWeight  = juce::Font::plain;
    constexpr int bigWaveformLabelSizePx = 11;
    constexpr int bigWaveformLabelWeight  = juce::Font::bold;
    constexpr int libraryHeaderSizePx = 11;
    constexpr int libraryHeaderWeight  = juce::Font::bold;
    constexpr int libraryRowSizePx = 11;
    constexpr int libraryRowWeight  = juce::Font::plain;
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

// Especificações do deck
namespace deckHeader {
    constexpr int heightPx = 44;
    constexpr int DecknumberWidthPx  = 7;
    constexpr int DecknumberHeightPx = 17;
    constexpr int TitleWidthPx  = 161;
    constexpr int TitleHeightPx = 18;
    constexpr int ArtistWidthPx  = 58;
    constexpr int ArtistHeightPx = 13;
    constexpr int BpmkeyWidthPx  = 55;
    constexpr int BpmkeyHeightPx = 13;
    constexpr int ElapsedtimeWidthPx  = 68;
    constexpr int ElapsedtimeHeightPx = 17;
    constexpr int RemainingtimeWidthPx  = 61;
    constexpr int RemainingtimeHeightPx = 17;
    constexpr int SyncWidthPx  = 39;
    constexpr int SyncHeightPx = 32;
    constexpr int CamelotkeyWidthPx  = 41;
    constexpr int CamelotkeyHeightPx = 32;
}
namespace deckTransport {
    constexpr int heightPx = 38;
}
namespace deckMiniWaveform {
    constexpr int widthPx  = 670;
    constexpr int heightPx = 34;
}
namespace deckCue {
    constexpr int widthPx  = 26;
    constexpr int heightPx = 26;
}
namespace deckPlay {
    constexpr int widthPx  = 26;
    constexpr int heightPx = 26;
}

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
    constexpr int diameterPx  = 102;
    constexpr int innerCirclePx = 88;
    constexpr int centerCirclePx = 64;
    namespace bpm {
        constexpr float widthPx  = 58.49f;
        constexpr float heightPx = 20.42f;
        constexpr int   sizePx   = 15;
        constexpr int   weight   = juce::Font::bold;
    }
    namespace pitch {
        constexpr float widthPx  = 52.0f;
        constexpr float heightPx = 11.0f;
        constexpr int   sizePx   = 9;
        constexpr int   weight   = juce::Font::plain;
    }
}
namespace pads {
    constexpr int cols = 4;
    constexpr int rows = 2;
    constexpr int total = 8;
}
namespace vumeter {
    constexpr int widthPx  = 2;
    constexpr int heightPx = 118;
    constexpr float minDb = -60;
    constexpr float maxDb = 0;
    constexpr int vumeterMarksDb[] = {
        0, -3, -6, -10, -16, -22, -32, -48,
    };
    constexpr int clipLedHeightPx = 4;
}
namespace eq {
    constexpr float minDb = -12.0f;
    constexpr float maxDb = 12.0f;
    constexpr float resolutionDb = 0.5f;
    struct Band { const char* name; const char* type; float frequencyHz; float q; };
    constexpr Band bands[] = {
        {"LOW", "lowShelf", 250.0f, 0.7f},
        {"MID", "peak", 1000.0f, 1.0f},
        {"HIGH", "highShelf", 4000.0f, 0.7f},
    };
}
namespace crossfader {
    constexpr float centerGain = 0.707f;
    namespace assignButton {
        constexpr int widthPx  = 12;
        constexpr int heightPx = 10;
    }
    namespace rail {
        constexpr int widthPx  = 100;
        constexpr int heightPx = 2;
    }
}
namespace volumeFader {
    namespace thumb {
        constexpr int widthPx  = 10;
        constexpr int heightPx = 7;
    }
    namespace rail {
        constexpr int widthPx  = 2;
        constexpr int heightPx = 46;
    }
}
namespace mixerCue {
    constexpr int widthPx  = 29.82;
    constexpr int heightPx = 14;
}

// Nós do Figma (profundidade 0 e 1) — fileKey W3Bup5XbRVyM5wH52Ir5Mn
namespace figmaCanvas {
    constexpr int  widthPx  = 1452;
    constexpr int heightPx = 813;
}

namespace figmaNodeframePrincipal {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 0;
    constexpr int  widthPx  = 1452;
    constexpr int heightPx = 813;
}

namespace figmaNodebigwaveform1 {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 0;
    constexpr int  widthPx  = 1452;
    constexpr int heightPx = 71;
}

namespace figmaNodebigwaveform2 {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 71;
    constexpr int  widthPx  = 1452;
    constexpr int heightPx = 66;
}

namespace figmaNodedeck1 {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 137;
    constexpr int  widthPx  = 671;
    constexpr int heightPx = 242;
}

namespace figmaNodedeck2 {
    constexpr int  xPx  = 781;
    constexpr int  yPx  = 137;
    constexpr int  widthPx  = 671;
    constexpr int heightPx = 242;
}

namespace figmaNodeminiwaveform1 {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 181;
    constexpr int  widthPx  = 670;
    constexpr int heightPx = 34;
}

namespace figmaNodeminiwaveform2 {
    constexpr int  xPx  = 781;
    constexpr int  yPx  = 181;
    constexpr int  widthPx  = 670;
    constexpr int heightPx = 34;
}

namespace figmaNodedeck1Number {
    constexpr int  xPx  = 6;
    constexpr int  yPx  = 139;
    constexpr int  widthPx  = 7;
    constexpr int heightPx = 17;
}

namespace figmaNodedeck1Title {
    constexpr int  xPx  = 71;
    constexpr int  yPx  = 145;
    constexpr int  widthPx  = 161;
    constexpr int heightPx = 18;
}

namespace figmaNodedeck1Artist {
    constexpr int  xPx  = 70;
    constexpr int  yPx  = 163;
    constexpr int  widthPx  = 58;
    constexpr int heightPx = 13;
}

namespace figmaNodedeck1BpmKey {
    constexpr int  xPx  = 236;
    constexpr int  yPx  = 163;
    constexpr int  widthPx  = 55;
    constexpr int heightPx = 13;
}

namespace figmaNodedeck1Timer {
    constexpr int  xPx  = 428;
    constexpr int  yPx  = 160;
    constexpr int  widthPx  = 122;
    constexpr int heightPx = 17;
}

namespace figmaNodedeck2Number {
    constexpr int  xPx  = 785;
    constexpr int  yPx  = 138;
    constexpr int  widthPx  = 9;
    constexpr int heightPx = 17;
}

namespace figmaNodedeck2Title {
    constexpr int  xPx  = 842;
    constexpr int  yPx  = 145;
    constexpr int  widthPx  = 49;
    constexpr int heightPx = 18;
}

namespace figmaNodedeck2Artist {
    constexpr int  xPx  = 842;
    constexpr int  yPx  = 162;
    constexpr int  widthPx  = 74;
    constexpr int heightPx = 16;
}

namespace figmaNodedeck2BpmKey {
    constexpr int  xPx  = 1007;
    constexpr int  yPx  = 163;
    constexpr int  widthPx  = 66;
    constexpr int heightPx = 15;
}

namespace figmaNodedeck2Timer {
    constexpr int  xPx  = 1201;
    constexpr int  yPx  = 161;
    constexpr int  widthPx  = 129;
    constexpr int heightPx = 18;
}

namespace figmaNodekeySync1 {
    constexpr int  xPx  = 570;
    constexpr int  yPx  = 146;
    constexpr int  widthPx  = 41;
    constexpr int heightPx = 32;
}

namespace figmaNodebeatSync1 {
    constexpr int  xPx  = 626;
    constexpr int  yPx  = 146;
    constexpr int  widthPx  = 39;
    constexpr int heightPx = 32;
}

namespace figmaNodekeySync2 {
    constexpr int  xPx  = 1344;
    constexpr int  yPx  = 144;
    constexpr int  widthPx  = 50;
    constexpr int heightPx = 32;
}

namespace figmaNodebeatSync2 {
    constexpr int  xPx  = 1402;
    constexpr int  yPx  = 144;
    constexpr int  widthPx  = 44;
    constexpr int heightPx = 32;
}

namespace figmaNodejog1 {
    constexpr int  xPx  = 535;
    constexpr float  yPx  = 226.62f;
    constexpr float  widthPx  = 102.12f;
    constexpr float heightPx = 102.12f;
}

namespace figmaNodejog2 {
    constexpr int  xPx  = 825;
    constexpr float  yPx  = 227.31f;
    constexpr float  widthPx  = 102.06f;
    constexpr float heightPx = 102.06f;
}

namespace figmaNodecue1 {
    constexpr int  xPx  = 500;
    constexpr float  yPx  = 275.14f;
    constexpr int  widthPx  = 26;
    constexpr int heightPx = 26;
}

namespace figmaNodeplay1 {
    constexpr int  xPx  = 500;
    constexpr float  yPx  = 306.4f;
    constexpr int  widthPx  = 26;
    constexpr int heightPx = 26;
}

namespace figmaNodecue2 {
    constexpr int  xPx  = 790;
    constexpr float  yPx  = 275.57f;
    constexpr int  widthPx  = 26;
    constexpr int heightPx = 26;
}

namespace figmaNodeplay2 {
    constexpr int  xPx  = 790;
    constexpr float  yPx  = 305.7f;
    constexpr int  widthPx  = 26;
    constexpr int heightPx = 26;
}

namespace figmaNodepitch_slider_1 {
    constexpr int  xPx  = 648;
    constexpr int  yPx  = 227;
    constexpr float  widthPx  = 9.94f;
    constexpr int heightPx = 103;
}

namespace figmaNodepitch_slider_2 {
    constexpr int  xPx  = 934;
    constexpr int  yPx  = 227;
    constexpr float  widthPx  = 9.94f;
    constexpr int heightPx = 103;
}

namespace figmaNodemixer {
    constexpr int  xPx  = 672;
    constexpr int  yPx  = 137;
    constexpr int  widthPx  = 109;
    constexpr int heightPx = 242;
}

namespace figmaNodemixerVu1 {
    constexpr float  xPx  = 682.8f;
    constexpr int  yPx  = 145;
    constexpr int  widthPx  = 2;
    constexpr int heightPx = 118;
}

namespace figmaNodemixerVu2 {
    constexpr float  xPx  = 734.48f;
    constexpr int  yPx  = 145;
    constexpr int  widthPx  = 2;
    constexpr int heightPx = 118;
}

namespace figmaNodemixerKnobTrim1 {
    constexpr float  xPx  = 692.74f;
    constexpr float  yPx  = 145.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobHigh1 {
    constexpr float  xPx  = 692.74f;
    constexpr float  yPx  = 178.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobMid1 {
    constexpr float  xPx  = 692.74f;
    constexpr float  yPx  = 211.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobLow1 {
    constexpr float  xPx  = 692.74f;
    constexpr float  yPx  = 244.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobTrim2 {
    constexpr float  xPx  = 744.42f;
    constexpr float  yPx  = 145.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobHigh2 {
    constexpr float  xPx  = 744.42f;
    constexpr float  yPx  = 178.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobMid2 {
    constexpr float  xPx  = 744.42f;
    constexpr float  yPx  = 211.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerKnobLow2 {
    constexpr float  xPx  = 744.42f;
    constexpr float  yPx  = 244.04f;
    constexpr int  widthPx  = 20;
    constexpr int heightPx = 20;
}

namespace figmaNodemixerCueBtn1 {
    constexpr float  xPx  = 687.77f;
    constexpr int  yPx  = 276;
    constexpr float  widthPx  = 29.82f;
    constexpr int heightPx = 14;
}

namespace figmaNodemixerCueBtn2 {
    constexpr float  xPx  = 739.45f;
    constexpr int  yPx  = 276;
    constexpr float  widthPx  = 29.82f;
    constexpr int heightPx = 14;
}

namespace figmaNodemixerVolRail1 {
    constexpr float  xPx  = 701.68f;
    constexpr int  yPx  = 293;
    constexpr int  widthPx  = 2;
    constexpr int heightPx = 46;
}

namespace figmaNodemixerVolBtn1 {
    constexpr float  xPx  = 697.71f;
    constexpr int  yPx  = 293;
    constexpr float  widthPx  = 9.94f;
    constexpr int heightPx = 7;
}

namespace figmaNodemixerVolRail2 {
    constexpr float  xPx  = 753.36f;
    constexpr int  yPx  = 293;
    constexpr int  widthPx  = 2;
    constexpr int heightPx = 46;
}

namespace figmaNodemixerVolBtn2 {
    constexpr float  xPx  = 749.39f;
    constexpr int  yPx  = 293;
    constexpr float  widthPx  = 9.94f;
    constexpr int heightPx = 7;
}

namespace figmaNodemixerCrossfaderRail1 {
    constexpr float  xPx  = 678.82f;
    constexpr int  yPx  = 358;
    constexpr float  widthPx  = 99.39f;
    constexpr int heightPx = 2;
}

namespace figmaNodemixerCrossfaderBtn1 {
    constexpr float  xPx  = 721.56f;
    constexpr int  yPx  = 354;
    constexpr float  widthPx  = 11.93f;
    constexpr int heightPx = 10;
}

namespace figmaNodebiblioteca {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 379;
    constexpr int  widthPx  = 1452;
    constexpr int heightPx = 434;
}

namespace figmaNodelibraryTree {
    constexpr int  xPx  = 0;
    constexpr int  yPx  = 379;
    constexpr int  widthPx  = 249;
    constexpr int heightPx = 434;
}

namespace figmaNodetrackTable {
    constexpr int  xPx  = 249;
    constexpr int  yPx  = 379;
    constexpr int  widthPx  = 1203;
    constexpr int heightPx = 434;
}

namespace figmaNodebigWaveform1Label {
    constexpr int  xPx  = 12;
    constexpr int  yPx  = 8;
    constexpr int  widthPx  = 200;
    constexpr int heightPx = 20;
}

namespace figmaNodebigWaveform2Label {
    constexpr int  xPx  = 12;
    constexpr int  yPx  = 8;
    constexpr int  widthPx  = 200;
    constexpr int heightPx = 20;
}
} // namespace layout
} // namespace djdeck

#endif // DJDECK_LAYOUT_CONFIG_HPP
