#!/usr/bin/env python3
"""
djdeck layout codegen.
Lê Source/layout.json e gera Source/LayoutConfig.hpp com constantes
constexpr / inline que podem ser usadas direto no C++ (JUCE) sem parsing
de JSON em runtime.

Uso:
    python tools/layout_codegen.py [Source/layout.json] [Source/LayoutConfig.hpp]

Se os caminhos forem omitidos, usa Source/layout.json e Source/LayoutConfig.hpp
relativamente à raiz do repositório (pasta que contem Source/).
"""

import json
import sys
from pathlib import Path

HEADER_GUARD = "DJDECK_LAYOUT_CONFIG_HPP"


def hex_to_juce_colour(hex_str):
    """Converte '#RRGGBB' para juce::Colour(0xffRRGGBB)."""
    h = hex_str.lstrip("#")
    return f"juce::Colour(0xff{h})"


def weight_to_juce(weight):
    """Converte nome de peso para o estilo JUCE Font."""
    if weight == "bold":
        return "juce::Font::bold"
    if weight == "regular":
        return "juce::Font::plain"
    return "juce::Font::plain"


def fmt_float(v):
    """Formata um valor como float literal C++ válido."""
    s = str(v)
    if '.' in s:
        return s + 'f'
    return s + '.0f'


def cpp_type_for_value(v):
    """Retorna o tipo C++ (int ou float) apropriado para o valor."""
    return "float" if isinstance(v, float) else "int"


def cpp_literal(v):
    """Formata o valor como literal C++ com sufixo quando for float."""
    if isinstance(v, float):
        return f"{v}f"
    return str(v)


def format_array(name, values, cpp_type="constexpr int"):
    lines = [f"{cpp_type} {name}[] = {{" ]
    lines.append("    " + ", ".join(str(v) for v in values) + ",")
    lines.append("};")
    return "\n".join(lines)


def generate_header(spec):
    out = []

    out.append(f"//==============================================================================")
    out.append(f"//")
    out.append(f"//  LayoutConfig.hpp — GERADO AUTOMATICAMENTE a partir de Source/layout.json")
    out.append(f"//  NÃO EDITAR À MÃO. Rode: python tools/layout_codegen.py")
    out.append(f"//")
    out.append(f"//  Constantes de UI centralizadas para djdeck (cores, proporções,")
    out.append(f"//  tipografia, regras de espelhamento). Sem parsing de JSON em runtime.")
    out.append(f"//")
    out.append(f"//==============================================================================")
    out.append(f"")
    out.append(f"#pragma once")
    out.append(f"")
    out.append(f"#ifndef {HEADER_GUARD}")
    out.append(f"#define {HEADER_GUARD}")
    out.append(f"")
    out.append(f"#include <juce_graphics/juce_graphics.h>")
    out.append(f"")
    out.append(f"namespace djdeck {{")
    out.append(f"namespace layout {{")

    # ---- screen ----
    screen = spec["screen"]
    out.append(f"")
    out.append(f"// Tela (px)")
    out.append(f"constexpr int screenWidth  = {screen['width']};")
    out.append(f"constexpr int screenHeight = {screen['height']};")

    # ---- sections ----
    out.append(f"")
    out.append(f"// Seções verticais (percentagem da altura da janela)")
    for s in spec["sections"]["vertical"]:
        ident = s["id"].replace("And", "And")
        out.append(f"constexpr int section{ident.capitalize()}HeightPercent = {s.get('heightPercent', 0)};")
        out.append(f"constexpr int section{ident.capitalize()}HeightPx        = {s['heightPx']};")

    # ---- decksAndMixer ----
    out.append(f"")
    out.append(f"// Colunas da seção Decks+Mixer (% da largura)")
    dm = spec["decksAndMixer"]["columns"]
    out.append(f"constexpr int deckAWidthPercent  = {dm['deckA']};")
    out.append(f"constexpr int mixerWidthPercent  = {dm['mixer']};")
    out.append(f"constexpr int deckBWidthPercent  = {dm['deckB']};")

    # Larguras/alturas absolutas dos decks (px)
    dmd = spec["decksAndMixer"]
    if "deckWidths" in dmd:
        out.append(f"")
        out.append(f"// Larguras absolutas dos decks (px, medida do Figma)")
        out.append(f"constexpr int deckAWidthPx  = {dmd['deckWidths']['deckA']};")
        out.append(f"constexpr int mixerWidthPx  = {dmd['deckWidths']['mixer']};")
        out.append(f"constexpr int deckBWidthPx  = {dmd['deckWidths']['deckB']};")
    if "deckHeights" in dmd:
        out.append(f"")
        out.append(f"// Alturas absolutas dos decks (px, medida do Figma)")
        out.append(f"constexpr int deckAHeightPx  = {dmd['deckHeights']['deckA']};")
        out.append(f"constexpr int mixerHeightPx  = {dmd['deckHeights']['mixer']};")
        out.append(f"constexpr int deckBHeightPx  = {dmd['deckHeights']['deckB']};")

    # ---- library ----
    out.append(f"")
    out.append(f"// Colunas da Biblioteca (% da largura)")
    lib = spec["library"]["columns"]
    out.append(f"constexpr int libraryTreeWidthPercent  = {lib['libraryTree']};")
    out.append(f"constexpr int trackListWidthPercent    = {lib['trackList']};")

    # ---- spacing ----
    out.append(f"")
    out.append(f"// Espaçamento (px)")
    sp = spec["spacing"]
    for key, val in sp.items():
        out.append(f"constexpr int {key} = {val};")

    # ---- colors ----
    out.append(f"")
    out.append(f"// Cores (valores medidos no Rekordbox — docs/DOCUMENTACAO_LAYOUT_UI.md §1)")
    out.append(f"namespace colours {{")
    col = spec["colors"]
    for group in ("surfaces", "text", "states", "djSemantics", "waveform", "mixer"):
        out.append(f"    // {group}")
        for name, hexval in col[group].items():
            cpp_name = "".join(w.capitalize() for w in name.split("_"))
            out.append(f"    inline const juce::Colour {cpp_name} = {hex_to_juce_colour(hexval)};")
    out.append(f"")
    out.append(f"    // Hot cue pads — 8 cores distintas")
    out.append(f"    inline const juce::Colour hotCue[8] = {{")
    for i, hexval in enumerate(col["hotCue"]):
        comma = "," if i < len(col["hotCue"]) - 1 else ""
        out.append(f"        {hex_to_juce_colour(hexval)}{comma}")
    out.append(f"    }};")
    out.append(f"}}")

    # ---- typography ----
    out.append(f"")
    out.append(f"// Tipografia (px)")
    out.append(f"namespace typography {{")
    tf = spec["typography"]
    if "fontFamily" in tf:
        out.append(f'    inline const char* fontFamily = "{tf["fontFamily"]}";')
    for role, spec_role in tf["roles"].items():
        out.append(f"    constexpr int {role}SizePx = {spec_role['sizePx']};")
        out.append(f"    constexpr int {role}Weight  = {weight_to_juce(spec_role['weight'])};")
    out.append(f"}}")

    # ---- mirroring ----
    out.append(f"")
    out.append(f"// Regras de espelhamento dos decks")
    out.append(f"constexpr bool mirroringEnabled = {str(spec['mirroring']['enabled']).lower()};")
    out.append(f"")
    mir = spec["mirroring"]["elements"]
    for elem, rule in mir.items():
        ident = "".join(w.capitalize() for w in elem.split("_"))
        out.append(f"// {rule['label']}")
        left_val = rule["mirrored=false"]
        right_val = rule["mirrored=true"]
        out.append(f"constexpr bool {ident}MirroredLeft  = {str(left_val == 'left').lower()};")
        out.append(f"constexpr bool {ident}MirroredRight = {str(right_val == 'left').lower()};")

    # ---- deck ----
    out.append(f"")
    out.append(f"// Especificações do deck")
    dk = spec["deck"]
    if "header" in dk:
        hdr = dk["header"]
        out.append(f"namespace deckHeader {{")
        if "heightPx" in hdr:
            out.append(f"    constexpr int heightPx = {hdr['heightPx']};")
        for el in hdr["elements"]:
            el_id = el["id"]
            cpp_name = "".join(w.capitalize() for w in el_id.split("_"))
            if "widthPx" in el:
                out.append(f"    constexpr int {cpp_name}WidthPx  = {el['widthPx']};")
            if "heightPx" in el:
                out.append(f"    constexpr int {cpp_name}HeightPx = {el['heightPx']};")
            if "xPx" in el:
                out.append(f"    constexpr int {cpp_name}Xpx = {el['xPx']};")
            if "yPx" in el:
                out.append(f"    constexpr int {cpp_name}Ypx = {el['yPx']};")
        out.append(f"}}")
    if "transport" in dk:
        out.append(f"namespace deckTransport {{")
        out.append(f"    constexpr int heightPx = {dk['transport']['heightPx']};")
        out.append(f"}}")
    if "miniWaveform" in dk:
        mw = dk["miniWaveform"]
        out.append(f"namespace deckMiniWaveform {{")
        out.append(f"    constexpr int widthPx  = {mw['widthPx']};")
        out.append(f"    constexpr int heightPx = {mw['heightPx']};")
        out.append(f"}}")
    if "cue" in dk:
        cue = dk["cue"]
        out.append(f"namespace deckCue {{")
        out.append(f"    constexpr int widthPx  = {cue['widthPx']};")
        out.append(f"    constexpr int heightPx = {cue['heightPx']};")
        out.append(f"}}")
    if "play" in dk:
        play = dk["play"]
        out.append(f"namespace deckPlay {{")
        out.append(f"    constexpr int widthPx  = {play['widthPx']};")
        out.append(f"    constexpr int heightPx = {play['heightPx']};")
        if "symbol" in play:
            out.append(f"    namespace symbol {{")
            out.append(f"        constexpr int widthPx  = {play['symbol']['widthPx']};")
            out.append(f"        constexpr int heightPx = {play['symbol']['heightPx']};")
            out.append(f"    }}")
        out.append(f"}}")
    if "stop" in dk:
        stop = dk["stop"]
        out.append(f"namespace deckStop {{")
        out.append(f"    constexpr int widthPx  = {stop['widthPx']};")
        out.append(f"    constexpr int heightPx = {stop['heightPx']};")
        out.append(f"}}")
    if "sync" in dk:
        sync = dk["sync"]
        out.append(f"namespace deckSync {{")
        out.append(f"    constexpr int widthPx  = {sync['widthPx']};")
        out.append(f"    constexpr int heightPx = {sync['heightPx']};")
        out.append(f"}}")
    if "camelotKey" in dk:
        ck = dk["camelotKey"]
        out.append(f"namespace deckCamelotKey {{")
        out.append(f"    constexpr int widthPx  = {ck['widthPx']};")
        out.append(f"    constexpr int heightPx = {ck['heightPx']};")
        out.append(f"}}")

    # ---- components ----
    out.append(f"")
    out.append(f"// Especificações de componentes")
    comp = spec["components"]

    out.append(f"namespace knob {{")
    k = comp["knob"]
    out.append(f"    constexpr float indicatorLengthPercent = {k['indicatorLengthPercent']};")
    out.append(f"    constexpr int   arcThicknessPx         = {k['arcThicknessPx']};")
    out.append(f"    constexpr float arcDegrees             = {k['arcDegrees']};")
    out.append(f"    constexpr int   labelSizePx            = {k['labelSizePx']};")
    out.append(f"    constexpr bool  labelBelow             = {str(k['labelBelow']).lower()};")
    out.append(f"}}")

    out.append(f"namespace slider {{")
    sv = comp["slider"]["vertical"]
    out.append(f"    namespace vertical {{")
    out.append(f"        constexpr int railWidthPx    = {sv['railWidthPx']};")
    out.append(f"        constexpr int handleHeightPx = {sv['handleHeightPx']};")
    out.append(f"    }}")
    sh = comp["slider"]["horizontal"]
    out.append(f"    namespace horizontal {{")
    out.append(f"        constexpr int railHeightPx  = {sh['railHeightPx']};")
    out.append(f"        constexpr int handleWidthPx = {sh['handleWidthPx']};")
    out.append(f"    }}")
    out.append(f"}}")

    def fmt_float(v):
        """Formata um valor como float literal C++ válido."""
        s = str(v)
        if '.' in s:
            return s + 'f'
        return s + '.0f'

    out.append(f"namespace jogWheel {{")
    jw = comp["jogWheel"]
    out.append(f"    constexpr int diameterPx  = {jw['diameterPx']};")
    out.append(f"    constexpr int innerCirclePx = {jw['innerCirclePx']};")
    out.append(f"    constexpr int centerCirclePx = {jw['centerCirclePx']};")
    if "bpm" in jw:
        out.append(f"    namespace bpm {{")
        out.append(f"        constexpr float widthPx  = {fmt_float(jw['bpm']['widthPx'])};")
        out.append(f"        constexpr float heightPx = {fmt_float(jw['bpm']['heightPx'])};")
        out.append(f"        constexpr int   sizePx   = {jw['bpm']['sizePx']};")
        out.append(f"        constexpr int   weight   = {weight_to_juce(jw['bpm']['weight'])};")
        out.append(f"    }}")
    if "pitch" in jw:
        out.append(f"    namespace pitch {{")
        out.append(f"        constexpr float widthPx  = {fmt_float(jw['pitch']['widthPx'])};")
        out.append(f"        constexpr float heightPx = {fmt_float(jw['pitch']['heightPx'])};")
        out.append(f"        constexpr int   sizePx   = {jw['pitch']['sizePx']};")
        out.append(f"        constexpr int   weight   = {weight_to_juce(jw['pitch']['weight'])};")
        out.append(f"    }}")
    out.append(f"}}")

    out.append(f"namespace pads {{")
    out.append(f"    constexpr int cols = 4;")
    out.append(f"    constexpr int rows = 2;")
    out.append(f"    constexpr int total = 8;")
    out.append(f"}}")

    out.append(f"namespace vumeter {{")
    vm = comp["vumeter"]
    if "widthPx" in vm:
        out.append(f"    constexpr int widthPx  = {vm['widthPx']};")
    if "heightPx" in vm:
        out.append(f"    constexpr int heightPx = {vm['heightPx']};")
    out.append(f"    constexpr float minDb = {vm['minDb']};")
    out.append(f"    constexpr float maxDb = {vm['maxDb']};")
    marks = format_array("vumeterMarksDb", vm["marksDb"])
    for line in marks.splitlines():
        out.append(f"    {line}")
    out.append(f"    constexpr int clipLedHeightPx = {vm['clipLedHeightPx']};")
    out.append(f"}}")

    out.append(f"namespace eq {{")
    eq = comp["eq"]
    out.append(f"    constexpr float minDb = {fmt_float(eq['rangeDb'][0])};")
    out.append(f"    constexpr float maxDb = {fmt_float(eq['rangeDb'][1])};")
    out.append(f"    constexpr float resolutionDb = {fmt_float(eq['resolutionDb'])};")
    out.append(f"    struct Band {{ const char* name; const char* type; float frequencyHz; float q; }};")
    out.append(f"    constexpr Band bands[] = {{")
    for b in eq["bands"]:
        out.append(f"        {{\"{b['name']}\", \"{b['type']}\", {b['frequencyHz']}.0f, {b['q']}f}},")
    out.append(f"    }};")
    out.append(f"}}")

    out.append(f"namespace crossfader {{")
    cf = comp["crossfader"]
    out.append(f"    constexpr float centerGain = {fmt_float(cf['centerGain'])};")
    if "assignButton" in cf:
        out.append(f"    namespace assignButton {{")
        out.append(f"        constexpr int widthPx  = {cf['assignButton']['widthPx']};")
        out.append(f"        constexpr int heightPx = {cf['assignButton']['heightPx']};")
        out.append(f"    }}")
    if "rail" in cf:
        out.append(f"    namespace rail {{")
        out.append(f"        constexpr int widthPx  = {cf['rail']['widthPx']};")
        out.append(f"        constexpr int heightPx = {cf['rail']['heightPx']};")
        out.append(f"    }}")
    out.append(f"}}")

    out.append(f"namespace volumeFader {{")
    vf = comp["volumeFader"]
    if "thumb" in vf:
        out.append(f"    namespace thumb {{")
        out.append(f"        constexpr int widthPx  = {vf['thumb']['widthPx']};")
        out.append(f"        constexpr int heightPx = {vf['thumb']['heightPx']};")
        out.append(f"    }}")
    if "rail" in vf:
        out.append(f"    namespace rail {{")
        out.append(f"        constexpr int widthPx  = {vf['rail']['widthPx']};")
        out.append(f"        constexpr int heightPx = {vf['rail']['heightPx']};")
        out.append(f"    }}")
    out.append(f"}}")

    out.append(f"namespace mixerCue {{")
    mc = comp["mixerCue"]
    # `mixerCue.widthPx` vale 29.82 px (medida fracionaria do Figma/HTML).
    # Declarar como `int` forcava a conversao double -> int e produzia o
    # warning C4244 em todos os arquivos que incluem este header.
    out.append(f"    constexpr {cpp_type_for_value(mc['widthPx'])}  widthPx  = {cpp_literal(mc['widthPx'])};")
    out.append(f"    constexpr int heightPx = {mc['heightPx']};")
    out.append(f"}}")

    # ---- figmaNodes ----
    out.append(f"")
    out.append(f"// Nós do Figma (profundidade 0 e 1) — fileKey {spec['figmaNodes']['fileKey']}")
    canvas = spec["figmaNodes"]["canvas"]
    out.append(f"namespace figmaCanvas {{")
    out.append(f"    constexpr {cpp_type_for_value(canvas['width'])}  widthPx  = {cpp_literal(canvas['width'])};")
    out.append(f"    constexpr {cpp_type_for_value(canvas['height'])} heightPx = {cpp_literal(canvas['height'])};")
    out.append(f"}}")

    for node in spec["figmaNodes"]["nodes"]:
        ident = "figmaNode" + node["id"].replace(":", "_")
        out.append(f"")
        out.append(f"namespace {ident} {{")
        out.append(f"    constexpr {cpp_type_for_value(node['x'])}  xPx  = {cpp_literal(node['x'])};")
        out.append(f"    constexpr {cpp_type_for_value(node['y'])}  yPx  = {cpp_literal(node['y'])};")
        out.append(f"    constexpr {cpp_type_for_value(node['width'])}  widthPx  = {cpp_literal(node['width'])};")
        out.append(f"    constexpr {cpp_type_for_value(node['height'])} heightPx = {cpp_literal(node['height'])};")
        out.append(f"}}")

    out.append(f"}} // namespace layout")
    out.append(f"}} // namespace djdeck")
    out.append(f"")
    out.append(f"#endif // {HEADER_GUARD}")

    return "\n".join(out) + "\n"


def main():
    repo_root = Path(__file__).resolve().parent.parent
    default_in = repo_root / "Source" / "layout.json"
    default_out = repo_root / "Source" / "LayoutConfig.hpp"

    in_path = Path(sys.argv[1]) if len(sys.argv) > 1 else default_in
    out_path = Path(sys.argv[2]) if len(sys.argv) > 2 else default_out

    if not in_path.exists():
        print(f"ERRO: não encontrado: {in_path}", file=sys.stderr)
        sys.exit(1)

    with open(in_path, "r", encoding="utf-8") as f:
        spec = json.load(f)

    header = generate_header(spec)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(header)

    print(f"Gerado: {out_path} (a partir de {in_path})")


if __name__ == "__main__":
    main()
