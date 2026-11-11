# Plano de Execução — Reformulação do Layout (djdeck)

**Data:** 2026-11-10
**Spec:** `docs/DOCUMENTACAO_LAYOUT_UI.md` §11.1 "Ordem sugerida de implementação"
**Base:** commit `ca4c4cd`, branch `main` (sem branch novo — decisão do usuário)
**Versionamento:** commit + tag só **depois** da aplicação completa e verificada

## Regra de ouro

> "não é para criar um branch, é para seguir com o versionamento"
> "seria versionado, apenas depois a aplicação das mudanças"

Trabalho acontece direto em `main`. Nenhum tag/commit intermediário. O
versionamento (commit + tag `v0.2.0`) é o **último** passo, após a Fase 10.

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

- [ ] `Source/RekordboxColours.h` — paleta da §1.3, nomes semânticos
- [ ] `Source/RekordboxLookAndFeel.h` / `.cpp` — `drawLinearSlider`,
      `drawButtonBackground`, `drawButtonText`, `drawRotarySlider`
- [ ] `MainComponent` aplica `RekordboxLookAndFeel` (substitui `LookAndFeel_V4`)
- [ ] LookAndFeel declarado como **último** member (§6 — ordem de destruição)

**Gate:** G1 + G2

## Fase 2 — Layout raiz (spec §11.1 item 2)

- [ ] `MainComponent::resized()` empilha 4 seções, na proporção da §2.1:
      header 10% / big waveforms 22% / decks+mixer 39% / biblioteca 28%
- [ ] `GlobalHeaderComponent` — barra de topo (modo, layout, módulos, relógio)
- [ ] Seções separadas por 1 px em `RekordboxColours::separator`

**Gate:** G1 + G3

## Fase 3 — Big waveform (spec §11.1 item 3)

- [ ] `Source/BigWaveformComponent.h` / `.cpp`
- [ ] Agulha **fixa no centro**; janela simétrica `position ± janela/2` (§4.1)
- [ ] `FileInputSource` no lugar do buffer integral (~212 MB/faixa → streaming)
- [ ] `BigWaveformsComponent` — 2 ondas empilhadas (deck 1 acima, deck 2 abaixo)
- [ ] Marcadores triangulares no topo/base da agulha

**Gate:** G1 + G2 + G3

## Fase 4 — DSP no DeckPlayer (spec §5.3–5.7, §5.10)

- [ ] `DeckPlayer::setTrimDb` / `getTrimDb` (±12 dB)
- [ ] `DeckPlayer::setEqLowDb` / `setEqMidDb` / `setEqHighDb` (±12 dB, shelf/peak/shelf)
- [ ] `DeckPlayer::setFilterAmount` (−1 passa-alta … +1 passa-baixa)
- [ ] `DeckPlayer::setCrossfadeGain` (multiplicado no ganho do canal)
- [ ] `DeckPlayer::applyGain` — trim × fader × crossfade
- [ ] `DeckPlayer::levelDb[2]` — `juce::Atomic<float>`, publicado já em dB
- [ ] Coeficientes de IIR trocados na fronteira de bloco (sem clique)

**Gate:** G1 + G4

## Fase 5 — Mixer (spec §11.1 item 5)

- [ ] `Source/KnobComponent.h` / `.cpp` — knob rotativo bipolar/unipolar
- [ ] `Source/VUMeterComponent.h` / `.cpp` — coluna vertical, gradiente, LED de clip
- [ ] `Source/ChannelStripComponent.h` / `.cpp` — VU + TRIM + HIGH/MID/LOW +
      FILTER + CUE + fader
- [ ] `Source/MixerComponent.h` / `.cpp` — 2 canais + área do crossfader
- [ ] Fader de canal com range linear 0–1 (mantém a API de `setGain`)

**Gate:** G1 + G3

## Fase 6 — Crossfader (spec §11.1 item 6)

- [ ] `Source/CrossfaderComponent.h` / `.cpp` — fader horizontal
- [ ] Curva **constant power**: `cos(t·π/2)` / `sin(t·π/2)`; centro = 0.707 cada
- [ ] Botões de atribuição **1** e **2** acima do fader
- [ ] Knobs **MIXING** e **LEVEL** no rodapé do mixer (§5.9)

**Gate:** G1 + G4

## Fase 7 — Jog wheel (spec §11.1 item 7)

- [ ] `Source/JogWheelComponent.h` / `.cpp` — `Component` customizado (não é Slider)
- [ ] Modos **touch** (arrasto circular → scratch) e **pitch** (arrasto vertical)
- [ ] BPM no centro (28 px), pitch % abaixo (13 px)
- [ ] `mirrored` só move os botões laterais, não o sentido da rotação

**Gate:** G1 + G3

## Fase 8 — Deck completo (spec §11.1 item 8)

- [ ] `Source/DeckHeaderComponent.h` / `.cpp` — nº do deck, título, artista,
      BPM, badge de key, tempo decorrido/restante, SYNC, MASTER
- [ ] `Source/HotCuePadsComponent.h` / `.cpp` — grade 4×2, cores `hotCue[8]`,
      letra + timestamp
- [ ] `Source/BeatJumpComponent.h` / `.cpp` — mesma grade, rótulos de fração
- [ ] `Source/DeckComponent.h` / `.cpp` — deck novo, espelhado via `bool mirrored`
- [ ] Deck 1 = Beat Jump à esquerda; deck 2 = Hot Cue à direita (§7.3)
- [ ] `WaveformDisplay` → `MiniWaveformComponent` (renomear; grade + cues + overview)

**Gate:** G1 + G2 + G3

## Fase 9 — Biblioteca em árvore (spec §11.1 item 9)

- [ ] `Source/LibraryComponent.h` / `.cpp` — árvore 30% + lista 70%
- [ ] `LibraryTreeModel` (`juce::TreeViewItem` — `paintItem`, não `paintTreeItem`)
- [ ] `StretchableLayoutManager` no lugar de `SplitterComponent` (não existe no JUCE 9)
- [ ] Nodos: Collection, Playlists, Explorer, Devices, Histories, Recordings

**Gate:** G1 + G3

## Fase 10 — Verificação final e documentação

- [ ] Build limpo do zero (sem cache incremental)
- [ ] App abre, responde e renderiza (§G2, §G3)
- [ ] Capturas de tela em `docs/screenshots/`
- [ ] `docs/DOCUMENTACAO_LAYOUT_UI.md` §11 atualizado — "Estado atual vs. alvo"
- [ ] `docs/ROADMAP.md` atualizado
- [ ] `Source/` sem `JuceHeader.h`, um componente por arquivo, sem estado global (§9.1)

**Gate:** G1 + G2 + G3 + G4

## Fase 11 — Versionamento (após tudo verificado)

- [ ] `git add` dos arquivos novos/alterados
- [ ] Commit único: `feat: reformular layout no modelo Rekordbox`
- [ ] `git tag v0.2.0` + `CMakeLists.txt` version bump para `0.2.0`
- [ ] Push para o remoto, se houver

---

## Fora de escopo (documentado, não implementado nesta rodada)

- **Detecção de BPM** — §11 "❌ ausente"; sem fonte de BPM, a grade de beats
  fica pronta mas o BPM exibido é placeholder. O ROADMAP decide o detector.
- **Metadados (TagLib)** — §11 "❌ ausente"; BPM/key/álbum/gênero da biblioteca
  ficam como placeholders até o TagLib entrar.
- **SYNC / MASTER reais** — os botões existem no cabeçalho; a sincronização
  depende do detector de BPM.
- **Hot Cue persistente** — os pads existem e guardam a posição em memória; a
  persistência em disco depende do formato de metadados.
- **PFL / saída de fone** — §5.9 exige uma segunda saída de áudio; a UI
  (CUE, MIXING, LEVEL) entra, o roteamento de áudio fica para depois.
