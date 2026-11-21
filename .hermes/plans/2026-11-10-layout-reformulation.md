# Plano de Execução — Reformulação do Layout (djdeck)

> **Documento histórico — concluído.** O plano foi executado em `main` entre as
> tags `v0.4.0` e `v0.5.0`. A implementação vigente e as pendências atuais
> estão documentadas em `docs/DOCUMENTACAO_LAYOUT_UI.md` e `docs/ROADMAP.md`.

**Data:** 2026-11-10
**Spec:** `docs/DOCUMENTACAO_LAYOUT_UI.md` §11.1 "Ordem sugerida de implementação"
**Base inicial:** commit `ca4c4cd`, branch `main` (sem branch novo — decisão do usuário)
**Release resultante:** `v0.5.0` (a versão prevista no plano original era `v0.2.0`, mas a base real já havia avançado para `v0.4.0`)
**Versionamento:** commit + tag somente após a aplicação completa e verificada

## Regra de ouro

> "não é para criar um branch, é para seguir com o versionamento"
> "seria versionado, apenas depois a aplicação das mudanças"

Trabalho acontece direto em `main`. Nenhum tag/commit intermediário. O
versionamento (commit + tag `v0.5.0`) foi o **último** passo, após as fases de
implementação e verificação.

## Gates de verificação

| Gate | Comando | Critério de aprovação |
|---|---|---|
| G1 Build | `cmake --build Builds --config Debug` | exit 0, sem warning novo |
| G2 App viva | abrir `Builds/djdeck_artefacts/Debug/djdeck.exe` | PID vivo, `MainWindowTitle=djdeck` |
| G3 Render | `tools/verify-window-render.ps1` | screenshot com ≥20 cores distintas |
| G4 Regressão de DSP | harness JUCE console (controle negativo) | exit != 0 quando a asserção falha |

O harness de teste (G4) é **pré-validado**: uma asserção deliberadamente
errada produziu `!!! Test 3 failed` e `EXIT=1`. Sem ele, o gate não prova nada.

---

## Fase 1 — Base visual (spec §11.1 item 1)

- [x] `Source/RekordboxColours.h` — paleta da §1.3, nomes semânticos
- [x] `Source/RekordboxLookAndFeel.h` / `.cpp` — `drawLinearSlider`,
      `drawButtonBackground`, `drawButtonText`, `drawRotarySlider`
- [x] `MainComponent` aplica `RekordboxLookAndFeel` (substitui `LookAndFeel_V4`)
- [x] LookAndFeel declarado como **último** member (§6 — ordem de destruição)

**Gate:** G1 + G2

## Fase 2 — Layout raiz (spec §11.1 item 2)

- [x] `MainComponent::resized()` posiciona big waveforms, decks+mixer e biblioteca
      conforme as medidas canônicas de `Source/layout.json`
- [x] `GlobalHeaderComponent` — barra de topo (modo, layout, módulos, relógio)
- [x] Seções separadas por 1 px em `RekordboxColours::separator`

**Gate:** G1 + G3

## Fase 3 — Big waveform (spec §11.1 item 3)

- [x] `Source/BigWaveformComponent.h` / `.cpp`
- [x] Agulha **fixa no centro**; janela simétrica `position ± janela/2` (§4.1)
- [x] `FileInputSource` no lugar do buffer integral (~212 MB/faixa → streaming)
- [x] `BigWaveformsComponent` — 2 ondas empilhadas (deck 1 acima, deck 2 abaixo)
- [x] Marcadores triangulares no topo/base da agulha

**Gate:** G1 + G2 + G3

## Fase 4 — DSP no DeckPlayer (spec §5.3–5.7, §5.10)

- [x] `DeckPlayer::setTrimDb` / `getTrimDb` (±12 dB)
- [x] `DeckPlayer::setEqLowDb` / `setEqMidDb` / `setEqHighDb` (±12 dB, shelf/peak/shelf)
- [x] `DeckPlayer::setFilterAmount` (−1 passa-alta … +1 passa-baixa)
- [x] `DeckPlayer::setCrossfadeGain` (multiplicado no ganho do canal)
- [x] `DeckPlayer::applyGain` — trim × fader × crossfade
- [x] `DeckPlayer::levelDb[2]` — `juce::Atomic<float>`, publicado já em dB
- [x] Coeficientes de IIR trocados na fronteira de bloco (sem clique)

**Gate:** G1 + G4

## Fase 5 — Mixer (spec §11.1 item 5)

- [x] `Source/KnobComponent.h` / `.cpp` — knob rotativo bipolar/unipolar
- [x] `Source/VUMeterComponent.h` / `.cpp` — coluna vertical, gradiente, LED de clip
- [x] `Source/ChannelStripComponent.h` / `.cpp` — VU + TRIM + HIGH/MID/LOW +
      FILTER + CUE + fader
- [x] `Source/MixerComponent.h` / `.cpp` — 2 canais + área do crossfader
- [x] Fader de canal com range linear 0–1 (mantém a API de `setGain`)

**Gate:** G1 + G3

## Fase 6 — Crossfader (spec §11.1 item 6)

- [x] `Source/CrossfaderComponent.h` / `.cpp` — fader horizontal
- [x] Curva **constant power**: `cos(t·π/2)` / `sin(t·π/2)`; centro = 0.707 cada
- [x] Botões de atribuição **1** e **2** acima do fader
- [x] Knobs **MIXING** e **LEVEL** no rodapé do mixer (§5.9)

**Gate:** G1 + G4

## Fase 7 — Jog wheel (spec §11.1 item 7)

- [x] `Source/JogWheelComponent.h` / `.cpp` — `Component` customizado (não é Slider)
- [x] Modos **touch** (arrasto circular → scratch) e **pitch** (arrasto vertical)
- [x] BPM no centro e pitch % abaixo
- [x] `mirrored` só move os botões laterais, não o sentido da rotação

**Gate:** G1 + G3

## Fase 8 — Deck completo (spec §11.1 item 8)

- [x] `Source/DeckHeaderComponent.h` / `.cpp` — nº do deck, capa, título, artista,
      BPM, tom, tempo decorrido/restante, SYNC, MASTER
- [x] `Source/HotCuePadsComponent.h` / `.cpp` — grade 4×2, cores `hotCue[8]`,
      letra + timestamp
- [x] `Source/BeatJumpComponent.h` / `.cpp` — mesma grade, rótulos de fração
- [x] `Source/DeckComponent.h` / `.cpp` — deck novo, espelhado via `bool mirrored`
- [x] Deck 1 = Beat Jump à esquerda; deck 2 = Hot Cue à direita (§7.3)
- [x] `WaveformDisplay` → `MiniWaveformComponent` (renomear; grade + cues + overview)

**Gate:** G1 + G2 + G3

## Fase 9 — Biblioteca em árvore (spec §11.1 item 9)

- [x] `Source/LibraryComponent.h` / `.cpp` — árvore de 249 px + lista de 1203 px
- [x] `LibraryTreeModel` (`juce::TreeViewItem` — `paintItem`, não `paintTreeItem`)
- [x] Larguras explícitas alinhadas ao Figma, sem `SplitterComponent`
- [x] Nodos: Collection, Playlists, Explorer, Devices, Histories, Recordings

**Gate:** G1 + G3

## Fase 10 — Verificação final e documentação

- [x] Build Debug e Release verificados
- [x] App abre, responde e renderiza (§G2, §G3)
- [x] Capturas de tela em `docs/screenshots/`
- [x] `docs/DOCUMENTACAO_LAYOUT_UI.md` §11 atualizado — "Estado atual vs. alvo"
- [x] `docs/ROADMAP.md` atualizado
- [x] `Source/` sem `JuceHeader.h`, um componente por arquivo, sem estado global (§9.1)

**Gate:** G1 + G2 + G3 + G4

## Fase 11 — Versionamento (após tudo verificado)

- [x] `git add` dos arquivos novos/alterados
- [x] Commits da reformulação e da release em `main`
- [x] `git tag v0.5.0` + `CMakeLists.txt` version bump para `0.5.0`
- [x] Push de `main` e da tag `v0.5.0` para o GitHub

---

## Fora de escopo / pendências após a reformulação

- **SYNC / MASTER funcionais** — os botões existem no cabeçalho, mas a
  sincronização real entre decks ainda precisa ser implementada.
- **Hot Cue persistente** — os pads existem e guardam a posição em memória; a
  persistência em disco depende do formato de metadados.
- **PFL / saída de fone** — §5.9 exige uma segunda saída de áudio; a UI
  (CUE, MIXING, LEVEL) entra, o roteamento de áudio fica para depois.

> A detecção de BPM e a integração do TagLib, antes listadas como pendências,
> foram concluídas em releases anteriores e agora fazem parte do estado atual.
