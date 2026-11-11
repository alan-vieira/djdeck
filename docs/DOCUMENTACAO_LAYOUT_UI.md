# djdeck — Guia de Estilo, Layout e Referências de Construção

Documento de referência para a reestruturação visual do djdeck e para a
construção dos componentes que ainda não existem (big waveform, mixer, jog
wheel, knobs, VU meter, biblioteca em árvore).

> **Status:** referência técnica. As seções "Estado atual" descrevem o código
> existente em `Source/` (verificado no commit `7d0b704`). As seções
> "Especificação" descrevem o alvo. Onde os dois divergem, a divergência está
> explícita.
>
> **Base técnica:** JUCE **9.0.5** (tag fixada no `CMakeLists.txt`).
> Todas as APIs citadas neste documento foram verificadas no código-fonte
> da JUCE em `Builds/_deps/juce-src/`. Quando a JUCE 9 removeu ou renomeou
> uma API, o documento diz o que usar no lugar — ver
> [§10 Migração da API JUCE 9](#10-migração-da-api-juce-9-armadilhas-conhecidas).

---

## Índice

1. [Paleta de cores](#1-paleta-de-cores)
2. [Divisão de layout](#2-divisão-de-layout)
3. [Tipografia](#3-tipografia)
4. [Componentes de áudio](#4-componentes-de-áudio)
   - [4.1 BigWaveform](#41-bigwaveform-agulha-fixa-no-centro)
   - [4.2 Waveform do deck](#42-waveform-do-deck-agulha-móvel-esquerda--direita)
5. [Mixer](#5-mixer)
   - [5.1 Visão geral](#51-visão-geral)
   - [5.2 Canal (ChannelStrip)](#52-canal-channelstrip)
   - [5.3 TRIM](#53-trim)
   - [5.4 EQ de 3 bandas](#54-eq-de-3-bandas)
   - [5.5 Filtro](#55-filtro)
   - [5.6 Fader de canal](#56-fader-de-canal)
   - [5.7 VU meter](#57-vu-meter)
   - [5.8 CUE / PFL](#58-cue--pfl)
   - [5.9 MIXING / LEVEL](#59-mixing--level)
   - [5.10 Crossfader](#510-crossfader)
6. [Elementos de interface](#6-elementos-de-interface)
   - [6.1 Knob](#61-knob)
   - [6.2 Slider](#62-slider)
   - [6.3 Jog wheel](#63-jog-wheel)
   - [6.4 Botões](#64-botões)
   - [6.5 Pads (Hot Cue / Beat Jump)](#65-pads-hot-cue--beat-jump)
7. [Deck](#7-deck)
8. [Biblioteca](#8-biblioteca)
9. [Arquitetura de componentes](#9-arquitetura-de-componentes)
10. [Migração da API JUCE 9](#10-migração-da-api-juce-9-armadilhas-conhecidas)
11. [Estado atual vs. alvo](#11-estado-atual-vs-alvo)
12. [Referências](#12-referências)

---

## 1. Paleta de cores

### 1.1 Cores medidas nas referências

Valores extraídos por amostragem de pixel das capturas do Rekordbox usadas
como referência. Não são estimativas visuais — são os valores efetivamente
presentes nos arquivos de imagem.

| Papel | Hex | Medido em |
|---|---|---|
| Fundo principal (janela) | `#191919` | biblioteca — 54% dos pixels |
| Fundo de painel (mixer, headers) | `#1A1A1A` | tela de performance |
| Fundo de área elevada (toolbar) | `#282828` | tela de performance |
| Fundo do mixer central | `#121212` | tela de performance |
| Fundo preto (grade de onda) | `#000000` | 62% dos pixels da big waveform |
| Texto primário | `#FFFFFF` | tela de performance |
| Texto secundário / inativo | `#CCCCCC` | tela de performance |
| Texto terciário | `#AEAEAE` | biblioteca — borda/scrollbar |
| Azul de destaque (ativo) | `#007DE1` | biblioteca — 158 px |
| Azul de seleção (fundo de linha) | `#052E64` | biblioteca — 371×10 px |
| Laranja (loop / marcação) | `#FF8C00` | tela de performance |
| Verde (texto selecionado) | `#158016` | biblioteca — 23 px |
| Vermelho (hot cue / key badge) | `#A63031` | biblioteca |
| Amarelo (rating preenchido) | `#DEAB38` / `#ECAE33` | biblioteca — 381 px |
| Amarelo (estrela) | `#FFD700` | tela de performance |

### 1.2 Cores da waveform

Duas famílias distintas, ambas medidas na captura da big waveform:

**Onda em camadas (bandas espectrais)** — a faixa superior da captura:

| Banda | Faixa de matiz | Cor representativa |
|---|---|---|
| High | 150–179° | `#3EB4B4` (ciano) |
| High/mid | 180–209° | `#54B4B5` → `#3AABE8` |
| Mid | 210–239° | `#36ABDE` (azul) |
| Low | 240–299° | `#A2A5F2` (índigo) |
| Marcação de região | — | `#805200` (âmbar) |

A estratificação vertical foi confirmada na coluna `x=900`: matiz 125° (verde)
no topo → 181° (ciano) no meio → 205° (azul) na base. **A cor varia
verticalmente dentro da mesma barra** — é isso que distingue a onda em camadas
da onda simples.

**Onda simples (min/max)** — a faixa inferior:

| Região | Cor |
|---|---|
| Início | `#00B4C2` (ciano) |
| Meio | `#00BE8F` (verde-azulado) |
| Fim | `#006EFF` → `#0078FF` (azul) |

### 1.3 Paleta de trabalho do djdeck

Constantes a criar em `Source/RekordboxColours.h`. Os nomes são semânticos,
não descritivos — trocar a paleta não deve tocar no código de desenho.

```cpp
#pragma once
#include <juce_graphics/juce_graphics.h>

namespace RekordboxColours
{
    // Superfícies
    inline const juce::Colour background     { juce::Colour(0xff191919) }; // #191919
    inline const juce::Colour panel          { juce::Colour(0xff1a1a1a) }; // #1A1A1A
    inline const juce::Colour panelRaised    { juce::Colour(0xff282828) }; // #282828
    inline const juce::Colour mixerBackground{ juce::Colour(0xff121212) }; // #121212
    inline const juce::Colour canvas         { juce::Colour(0xff000000) }; // #000000
    inline const juce::Colour separator      { juce::Colour(0xff323232) }; // #323232

    // Texto
    inline const juce::Colour textPrimary    { juce::Colour(0xffffffff) }; // #FFFFFF
    inline const juce::Colour textSecondary  { juce::Colour(0xffcccccc) }; // #CCCCCC
    inline const juce::Colour textTertiary   { juce::Colour(0xffaeaeae) }; // #AEAEAE
    inline const juce::Colour textDisabled   { juce::Colour(0xff707070) }; // #707070

    // Estados
    inline const juce::Colour accent         { juce::Colour(0xff007de1) }; // #007DE1
    inline const juce::Colour selection      { juce::Colour(0xff052e64) }; // #052E64
    inline const juce::Colour selectionText  { juce::Colour(0xff158016) }; // #158016
    inline const juce::Colour hover          { juce::Colour(0xff2a2a2a) };

    // Semânticas de DJ
    inline const juce::Colour loop           { juce::Colour(0xffff8c00) }; // #FF8C00
    inline const juce::Colour cue            { juce::Colour(0xffa63031) }; // #A63031
    inline const juce::Colour rating         { juce::Colour(0xffdeab38) }; // #DEAB38
    inline const juce::Colour beatGrid       { juce::Colour(0xffb8b8b8) }; // beat fino
    inline const juce::Colour downbeat       { juce::Colour(0xffa30000) }; // beat grosso
    inline const juce::Colour needle         { juce::Colour(0xffffffff) }; // agulha

    // Waveform
    inline const juce::Colour waveLow        { juce::Colour(0xff006eff) }; // #006EFF
    inline const juce::Colour waveMid        { juce::Colour(0xff00b4c2) }; // #00B4C2
    inline const juce::Colour waveHigh       { juce::Colour(0xff3eb4b4) }; // #3EB4B4
    inline const juce::Colour waveRegion     { juce::Colour(0xff805200) }; // #805200

    // Hot cue pads — 8 cores distintas
    inline const juce::Colour hotCue[8] =
    {
        juce::Colour(0xff007aff), // A azul
        juce::Colour(0xff00ff00), // B verde
        juce::Colour(0xffaf7ac7), // C roxo
        juce::Colour(0xffff8c00), // D laranja
        juce::Colour(0xff00b4c2), // E teal
        juce::Colour(0xffff6ec7), // F rosa
        juce::Colour(0xffffd700), // G amarelo
        juce::Colour(0xffff0000)  // H vermelho
    };
}
```

### 1.4 Gradientes

Gradientes usados com frequência, para não repetir a construção em cada
componente:

```cpp
// VU meter: verde -> amarelo -> vermelho
const juce::ColourGradient vuGradient =
    juce::ColourGradient(juce::Colours::yellow, 0.0f, 0.0f,
                         juce::Colours::orangered, 0.0f, 1.0f, false);

// Onda em camadas: base -> topo
const juce::ColourGradient waveGradient =
    juce::ColourGradient(RekordboxColours::waveLow, 0.0f, 1.0f,
                         RekordboxColours::waveHigh, 0.0f, 0.0f, false);
```

> `ColourGradient` está em `juce_graphics/colour/juce_ColourGradient.h`.
> O construtor é `ColourGradient(Colour c1, float x1, float y1, Colour c2, float x2, float y2, bool radial)`.
> Para acrescentar paradas use `addColour(double proportion, Colour)`.

---

## 2. Divisão de layout

### 2.1 Proporções da tela

Medido na captura do Rekordbox em modo PERFORMANCE / 2Deck Horizontal.
A janela do djdeck é `1600×900` (`MainComponent::MainComponent`, linha 51).

| Seção | Altura | % | Conteúdo |
|---|---|---|---|
| Header + barra FX | ~90 px | ~10% | modo, layout, módulos ativos, relógio |
| Big waveforms | ~200 px | ~22% | 2 ondas empilhadas (deck 1 acima, deck 2 abaixo) |
| Decks + mixer | ~350 px | ~39% | deck A \| mixer \| deck B |
| Biblioteca | ~250 px | ~28% | árvore à esquerda, lista de faixas à direita |

Na largura da seção de decks:

| Coluna | % da largura |
|---|---|
| Deck A | ~45% |
| Mixer | ~10% |
| Deck B | ~45% |

Na seção da biblioteca:

| Coluna | % da largura |
|---|---|
| Árvore | ~30% |
| Lista de faixas | ~70% |

### 2.2 Estrutura de componentes

Proposta de árvore de componentes. Os arquivos marcados com *(novo)* ainda não
existem.

```
MainComponent (juce::AudioAppComponent)
├── RekordboxLookAndFeel.h                    (novo)
├── GlobalHeaderComponent                     (novo)
├── BigWaveformsComponent                     (novo)
│   ├── BigWaveformComponent  (deck 1)
│   └── BigWaveformComponent  (deck 2)
├── DecksMixerComponent                       (novo)
│   ├── DeckComponent          (deck A)
│   │   ├── DeckHeaderComponent
│   │   ├── MiniWaveformComponent  (existente: WaveformDisplay, a renomear)
│   │   ├── BeatJumpComponent  (novo)  ┐ espelhados:
│   │   ├── HotCuePadsComponent (novo)  ┘ deck 1 = Beat Jump, deck 2 = Hot Cue
│   │   └── JogWheelComponent   (novo)
│   ├── MixerComponent                     (novo)
│   │   ├── ChannelStripComponent (deck A)
│   │   │   ├── VUMeterComponent
│   │   │   ├── KnobComponent (TRIM)
│   │   │   ├── KnobComponent (HIGH)
│   │   │   ├── KnobComponent (MID)
│   │   │   ├── KnobComponent (LOW)
│   │   │   ├── KnobComponent (FILTER)
│   │   │   ├── CUEButton
│   │   │   └── ChannelFader
│   │   ├── CrossfaderComponent
│   │   └── ChannelStripComponent (deck B)   (mesma estrutura, espelhada)
│   └── DeckComponent          (deck B)
└── LibraryComponent                       (novo)
    ├── LibraryTreeView + LibraryTreeModel
    ├── TrackListView + TrackListModel
    └── RelatedTracksPanel     (opcional)
```

### 2.3 Espelhamento dos decks

O Rekordbox espelha deck 1 e deck 2: o **Beat Jump** fica à esquerda no deck 1
e à direita no deck 2; o **Hot Cue** faz o inverso; o **jog wheel** troca de
lado; os rótulos de EQ ficam à direita no deck 1 e à esquerda no deck 2.

O djdeck implementa isso com um parâmetro `bool mirrored` passado ao
construtor do `DeckComponent`. Nenhum `if (mirrored)` espalhado pelo código
de desenho — o componente se posiciona a partir do parâmetro.

```cpp
// Exemplo: o knob de EQ rotula à direita no deck 1, à esquerda no deck 2
const auto labelSide = mirrored ? juce::Rectangle<int>::leftSide
                                : juce::Rectangle<int>::rightSide;
```

### 2.4 Espaçamento

| Token | Valor | Uso |
|---|---|---|
| `gapTiny` | 2 px | entre barras de onda |
| `gapSmall` | 4 px | entre pads |
| `gapMedium` | 8 px | entre knobs empilhados |
| `gapLarge` | 16 px | entre blocos do deck |

Margem interna dos painéis: 8 px. Borda entre seções: 1 px em
`RekordboxColours::separator`.

---

## 3. Tipografia

O djdeck hoje usa a fonte padrão do sistema. O `ROADMAP.md` prevê fonte
customizada open source. O material de referência (`Scratch/GUI-Components.h`,
adaptado do repositório `jcurtis4207/Juce-Plugins`, licença MIT) usa a
**XXIIAven** carregada via `BinaryData`:

```cpp
const juce::Font xxii { juce::Typeface::createSystemTypefaceFor (
                            BinaryData::XXIIAvenRegular_ttf,
                            BinaryData::XXIIAvenRegular_ttfSize) };
```

### 3.1 Escala tipográfica

| Papel | Tamanho | Peso | Uso |
|---|---|---|---|
| Display | 28 px | regular | BPM no centro do jog wheel |
| Title | 16 px | regular | título da faixa no deck |
| Body | 13 px | regular | rótulos de knob, BPM, key |
| Caption | 11 px | regular | timestamps, contadores |
| Micro | 9 px | regular | marcações da grade de beats |

Números devem usar algarismos tabulares para não tremer durante a reprodução.
A JUCE não expõe isso diretamente — o truque é formatar com largura fixa:

```cpp
// "  1:23" em vez de "1:23" — mantém a largura estável
juce::String(juce::String(mins) + ":" + juce::String(secs).paddedToLeft(2, '0'))
    .paddedToLeft(5, ' ');
```

### 3.2 Labels do mixer

Rótulos em `textSecondary`, centralizados abaixo do controle:

```cpp
g.setFont (juce::Font (12.0f));
g.setColour (RekordboxColours::textSecondary);
g.drawText ("TRIM", bounds.removeFromBottom (14), juce::Justification::centred, true);
```

---

## 4. Componentes de áudio

Os dois componentes de onda do djdeck têm **comportamentos opostos** e não
devem ser confundidos:

| | BigWaveform | Waveform do deck |
|---|---|---|
| Onde | topo da tela, 2 empilhadas | dentro do deck, sobre os controles |
| Agulha | **fixa no centro** | **móvel**, da esquerda para a direita |
| Onda | rola; início cravado na agulha | **parada**; faixa inteira visível |
| Zoom | janela deslizante | faixa completa |
| Propósito | leitura de posição à distância | precisão de posição e edição |

### 4.1 BigWaveform (agulha fixa no centro)

A agulha nunca se move. A onda desliza por baixo dela. No instante em que a
faixa começa a tocar, o **início da faixa está exatamente sob a agulha** — a
metade esquerda da área está vazia, e o material que vem a seguir entra pela
direita. Conforme o tempo avança, tudo escorrega para a esquerda.

Isso foi confirmado por medição de pixel na captura de referência: existe uma
coluna vertical contínua em `x=570` numa imagem de 1143 px de largura
(centro = 571.5), atravessando 107 das 111 linhas. É a agulha.

#### Modelo matemático

Com a agulha fixa no centro e a onda rolando, a janela de tempo visível é
**simétrica em torno da posição atual**:

```
visível = [ posição - janela/2 , posição + janela/2 ]
```

- `posição` = posição de playback em segundos
- `janela` = duração total visível na largura do componente (ex.: 30 s)

Como a janela é simétrica, `position` cai exatamente no pixel central — e a
agulha é desenhada em `width / 2`, sem depender do cálculo da janela.

No instante `posição = 0`, a janela é `[-janela/2, +janela/2]`: a metade
esquerda cai antes do início da faixa e fica vazia, e a metade direita contém
os primeiros `janela/2` segundos. **É exatamente o "início cravado na
agulha"**.

#### Implementação

`juce::AudioThumbnail::drawChannel` mapeia o intervalo `[startTime, endTime]`
na largura inteira da área — uma única chamada já produz a janela simétrica.

```cpp
void BigWaveformComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    g.fillAll (RekordboxColours::canvas);

    if (! fileLoaded)
        return;

    const auto windowSeconds = 30.0;                    // duração visível
    const auto position = currentPositionSeconds;

    // Janela simétrica em torno da posição: o ponto central do retângulo
    // corresponde exatamente ao instante `position`.
    const auto startTime = position - windowSeconds * 0.5;
    const auto endTime   = position + windowSeconds * 0.5;

    g.setColour (RekordboxColours::waveMid);
    audioThumb.drawChannel (g, area, startTime, endTime, 0, 1.0f);

    drawNeedle (g, area);
}

void BigWaveformComponent::drawNeedle (juce::Graphics& g,
                                       const juce::Rectangle<int>& area)
{
    const auto needleX = area.getCentreX();

    g.setColour (RekordboxColours::needle);
    g.drawLine (needleX, area.getY(), needleX, area.getBottom(), 1.0f);

    // Marcadores triangulares no topo e na base, como no Rekordbox
    juce::Path marker;
    marker.addTriangle (needleX - 4.0f, area.getY(),
                        needleX + 4.0f, area.getY(),
                        needleX,         area.getY() + 6.0f);
    g.fillPath (marker);
}
```

**Tempo negativo é seguro.** O `AudioThumbnail` ignora amostras anteriores ao
início do arquivo: em `AudioThumbnailWindow::refillCache` o teste `if (sample
>= 0)` descarta os pixels fora da faixa, deixando-os sem valor — e
`MinMaxValue::isNonZero()` falso impede que sejam desenhados. A metade
esquerda fica simplesmente vazia, sem nenhum tratamento especial.

> A API de `AudioThumbnail` em JUCE 9.0.5:
> ```cpp
> void setSource (const AudioBuffer<float>* newSource, double sampleRate, int64 hashCode);
> void setSource (InputSource* newSource);                    // de AudioThumbnailBase
> double getTotalLength() const noexcept;
> int getNumChannels() const noexcept;
> void drawChannel (Graphics&, const Rectangle<int>&, double startTimeSeconds,
>                   double endTimeSeconds, int channelNum, float verticalZoomFactor);
> void drawChannels (Graphics&, const Rectangle<int>&, double startTimeSeconds,
>                    double endTimeSeconds, float verticalZoomFactor);
> ```

#### Carregamento do áudio — não carregar o arquivo inteiro em memória

O `WaveformDisplay` atual (`Source/WaveformDisplay.cpp`, linha 79) carrega a
faixa **inteira** em um `juce::AudioBuffer<float>`:

```cpp
// Estado atual — problema: 10 min a 44.1 kHz estéreo = ~212 MB por faixa
waveformBuffer.setSize ((int) reader->numChannels,
                         (int) reader->lengthInSamples,
                         false, true, false);
reader->read (&waveformBuffer, 0, waveformBuffer.getNumSamples(), 0, true, true);
audioThumb.setSource (&waveformBuffer, reader->sampleRate, file.hashCode64());
```

O `AudioThumbnail` aceita um `InputSource*` e faz a leitura em background
thread, sem manter o áudio na memória. Trocar por `FileInputSource`:

```cpp
#include <juce_core/streams/juce_FileInputSource.h>

void BigWaveformComponent::loadURL (const juce::URL& url)
{
    const juce::File file = url.getLocalFile();
    if (! file.existsAsFile())
        return;

    fileLoaded = false;
    position = 0.0;

    // O thumbnail assume a posse do source e o destrói quando substituído.
    // FileInputSource abre o arquivo só quando precisa — não mantém handle
    // aberto o tempo todo.
    audioThumb.setSource (new juce::FileInputSource (file));

    fileLoaded = true;
    repaint();
}
```

> `AudioThumbnail::setSource(const AudioBuffer<float>*, ...)` **não copia** o
> buffer — o ponteiro precisa continuar válido. `setSource(InputSource*)` não
> tem essa restrição. `juce::FileInputSource` está em
> `juce_core/streams/juce_FileInputSource.h`:
> `FileInputSource(const File& file, bool useFileTimeInHashGeneration = false)`.

#### Grade de beats

A grade é desenhada **sobre** a onda, em vermelho para os downbeats e branco
fino para as batidas. As cores medidas na referência: `#A30000` (downbeat) e
`#B8B8B8` (batida).

```cpp
void BigWaveformComponent::drawBeatGrid (juce::Graphics& g,
                                         const juce::Rectangle<int>& area)
{
    if (beatGrid.empty() || windowSeconds <= 0.0)
        return;

    const auto pxPerSecond = area.getWidth() / windowSeconds;
    const auto startTime = currentPositionSeconds - windowSeconds * 0.5;

    // Batidas finas
    g.setColour (RekordboxColours::beatGrid.withAlpha (0.35f));
    for (const auto& beat : beatGrid)
    {
        if (! beat.isDownbeat)
        {
            const auto x = area.getX() + (float) ((beat.timeSeconds - startTime) * pxPerSecond);
            if (x >= area.getX() && x < area.getRight())
                g.drawLine ((int) x, area.getY(), (int) x, area.getBottom(), 1.0f);
        }
    }

    // Downbeats — mais grossos
    g.setColour (RekordboxColours::downbeat);
    for (const auto& beat : beatGrid)
    {
        if (beat.isDownbeat)
        {
            const auto x = area.getX() + (float) ((beat.timeSeconds - startTime) * pxPerSecond);
            if (x >= area.getX() && x < area.getRight())
                g.drawLine ((int) x, area.getY(), (int) x, area.getBottom(), 2.0f);
        }
    }
}
```

#### Marcadores de cue e loop

Triângulos coloridos na borda superior, fora da área da onda:

- **Hot cue** — 8 cores de `RekordboxColours::hotCue[]`
- **Loop** — laranja `#FF8C00`, desenhado como barra horizontal com marcador
  de início e fim

### 4.2 Waveform do deck (agulha móvel, esquerda → direita)

Este é o comportamento **oposto**: a faixa inteira fica visível e parada, e a
agulha percorre da esquerda para a direita acompanhando a reprodução.

O `WaveformDisplay` atual **já implementa esse comportamento**:

```cpp
// Source/WaveformDisplay.cpp — estado atual, já correto para esta spec
g.setColour (juce::Colours::orange);
audioThumb.drawChannel (g, area, 0.0, audioThumb.getTotalLength(), 0, 1.0f);

const auto playheadX = static_cast<float>(position) * static_cast<float>(area.getWidth());
g.setColour (juce::Colours::white);
g.drawLine (playheadX, area.getY(), playheadX, area.getBottom(), 2.0f);
```

O que falta para casar com a referência:

| Item | Estado | Nota |
|---|---|---|
| Faixa completa estática | ✅ presente | `drawChannel` de 0 ao total |
| Agulha esquerda → direita | ✅ presente | `position` relativo |
| Cor da onda | ⚠️ laranja chapado | usar gradiente `waveGradient` |
| Grade de beats | ❌ ausente | mesma rotina da big waveform |
| Marcadores de cue na borda | ❌ ausente | reaproveitar `hotCue[]` |
| Overview (miniatura) | ❌ ausente | faixa fina acima, com posição |
| Agulha com marcadores triangulares | ❌ ausente | |

O `position` é atualizado pelo timer de 33 ms do `DeckGUI`
(`Source/DeckGUI.cpp`, `timerCallback()`), que já lê
`player->getPositionRelative()`.

---

## 5. Mixer

### 5.1 Visão geral

O mixer do djdeck hoje **não existe como componente**. O que existe é:

- `juce::MixerAudioSource mixerSource` em `MainComponent` — soma as saídas
  dos dois decks (`MainComponent.cpp`, linha 59)
- `DeckPlayer::setGain()` — ganho linear 0.0–1.0 aplicado no
  `AudioTransportSource` (`DeckPlayer.cpp`, linha 61)

**Limitação importante:** `juce::MixerAudioSource` **não tem controle de
ganho por fonte** — a busca por "gain" no cabeçalho da JUCE 9.0.5 retorna
zero ocorrências. O crossfader não pode ser implementado no `MixerAudioSource`.
Ele precisa ser aplicado **antes** da soma, dentro de cada `DeckPlayer`.

Isso é vantajoso para o desenho: o `DeckPlayer` já é a unidade de ganho
natural. A cadeia de áudio por deck fica:

```
DeckPlayer
  AudioFormatReaderSource
    → AudioTransportSource        (transporte + pitch)
      → ResamplingAudioSource     (resample ratio)
        → [ Gain do canal ]       ← fader de volume
          → [ EQ 3 bandas ]       ← low/mid/high
            → [ Filtro ]          ← low/high pass
              → [ Ganho do crossfader ]  ← crossfader
                → saída para o MixerAudioSource
```

O sinal de um deck **não** deve entrar no `MixerAudioSource` antes de ter
passado por TRIM/EQ/filtro.

### 5.2 Canal (ChannelStrip)

A ordem vertical dentro de cada canal, de cima para baixo:

| # | Controle | Tipo | Faixa | Neutro |
|---|---|---|---|---|
| 1 | VU meter | vertical, com LED de clip | — | — |
| 2 | TRIM | knob | ±12 dB | centro = 0 dB |
| 3 | HIGH | knob | ±12 dB | centro = flat |
| 4 | MID | knob | ±12 dB | centro = flat |
| 5 | LOW | knob | ±12 dB | centro = flat |
| 6 | FILTER | knob | passa-baixa/alta | centro = neutro |
| 7 | CUE | botão quadrado | PFL | apagado |
| 8 | Fader de canal | slider vertical | 0 → 1 | topo = volume máximo |

O VU meter fica **ao lado** dos knobs, não acima — é uma coluna estreita
à esquerda do strip.

### 5.3 TRIM

Ganho de entrada do canal, aplicado antes de qualquer DSP. Existe para
compensar volumes diferentes entre faixas — não é um controle criativo.

```cpp
// Faixa do slider, em dB
trimSlider.setRange (-12.0f, 12.0f, 0.1f);
trimSlider.setValue (0.0f);          // centro = unity

// Duplo clique volta ao neutro.
// A assinatura é (habilitar, valor, modificadores) — o bool vem PRIMEIRO.
trimSlider.setDoubleClickReturnValue (true, 0.0);

// Conversão para o ganho linear do AudioTransportSource
void DeckPlayer::setTrimDb (double db)
{
    trimDb = juce::jlimit (-12.0, 12.0, db);
    applyGain();
}

void DeckPlayer::applyGain()
{
    // gain linear total = fader × trim
    const auto linear = juce::Decibels::decibelsToGain (trimDb)
                      * juce::jlimit (0.0f, 1.0f, (float) channelGain);
    transportSource.setGain (linear);
}
```

> `juce::Decibels::decibelsToGain<Type>(decibels, mindB)` e
> `gainToDecibels<Type>(gain, mindB)` estão em
> `juce_audio_basics/utilities/juce_Decibels.h`.

O TRIM é um **ganho** — deve ser aplicado linearmente ao sinal, não em dB.
A conversão dB→linear já produz o fator multiplicativo correto.

### 5.4 EQ de 3 bandas

EQ de três bandas: **low shelf**, **peak (mid)** e **high shelf**.

| Banda | Tipo | Frequência | Q |
|---|---|---|---|
| LOW | shelf | 250 Hz | 0.7 |
| MID | peak | 1 kHz | 1.0 |
| HIGH | shelf | 4 kHz | 0.7 |

Faixa: ±12 dB, resolução de 0.5 dB.

#### Implementação com IIR

`juce::IIRCoefficients` está no namespace `juce` (não `juce::IIR`), em
`juce_audio_basics/utilities/juce_IIRFilter.h`. As factories de shelf e peak
recebem **ganho como fator linear**, não em dB:

```cpp
// makeLowShelf (sampleRate, cutOffFrequency, Q, gainFactor)
// makePeakFilter (sampleRate, centreFrequency, Q, gainFactor)
// makeHighShelf (sampleRate, cutOffFrequency, Q, gainFactor)
//
// gainFactor > 1.0 -> boost ; < 1.0 -> attenuate
```

Conversão de dB para o fator linear:

```cpp
const auto gainFactor = juce::Decibels::decibelsToGain (12.0f);   // ~3.98
const auto unity      = juce::Decibels::decibelsToGain (0.0f);    // 1.0
```

Classe do filtro: `juce::IIRFilter` (com `SpinLock`, segura para uso
multithread) ou `juce::SingleThreadedIIRFilter` (mais leve, apenas para a
thread de áudio).

> **Atenção ao namespace:** em `juce_audio_basics` o filtro é
> **`juce::IIRFilter`** — namespace `juce`, não `juce::IIR`. O
> `juce::dsp::IIR::Filter` é outro tipo, em `juce_dsp`, que usa
> `CoefficientsPtr` (smart pointer) em vez de `IIRCoefficients` por valor.
> Os dois existem; este documento usa o de `juce_audio_basics` porque é mais
> simples e não exige alocação.

```cpp
class ThreeBandEq
{
public:
    void prepare (double sampleRate)
    {
        currentSampleRate = sampleRate;

        lowFilter.setCoefficients (juce::IIRCoefficients::makeLowShelf (
            sampleRate, 250.0, 0.7f, 1.0f));

        midFilter.setCoefficients (juce::IIRCoefficients::makePeakFilter (
            sampleRate, 1000.0, 1.0f, 1.0f));

        highFilter.setCoefficients (juce::IIRCoefficients::makeHighShelf (
            sampleRate, 4000.0, 0.7f, 1.0f));
    }

    void setLowGainDb (float db)
    {
        lowGainDb = db;
        lowFilter.setCoefficients (juce::IIRCoefficients::makeLowShelf (
            currentSampleRate, 250.0, 0.7f,
            juce::Decibels::decibelsToGain (db)));
    }

    void setMidGainDb (float db)  { /* makePeakFilter 1000.0, 1.0f */ }
    void setHighGainDb (float db) { /* makeHighShelf 4000.0, 0.7f */ }

    void reset()
    {
        lowFilter.reset();
        midFilter.reset();
        highFilter.reset();
    }

    // Processa um buffer in-place
    void process (juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* samples = buffer.getWritePointer (ch);
            const auto num = buffer.getNumSamples();

            lowFilter.processSamples (samples, num);
            midFilter.processSamples (samples, num);
            highFilter.processSamples (samples, num);
        }
    }

private:
    double currentSampleRate = 44100.0;
    float lowGainDb = 0.0f, midGainDb = 0.0f, highGainDb = 0.0f;

    juce::IIRFilter lowFilter;
    juce::IIRFilter midFilter;
    juce::IIRFilter highFilter;
};
```

> `juce::IIRFilter` herda de `IIRFilterBase<SpinLock>` e oferece
> `setCoefficients(const IIRCoefficients&)`, `processSamples(float*, int)` e
> `reset()`. `juce::IIRCoefficients` é uma classe simples com
> `float coefficients[5]` — copie por valor, nunca por referência.

**Os coeficientes só podem ser trocados quando o áudio não está passando
por eles.** Trocar coeficientes no meio de um bloco produz um clique. A
solução padrão: manter o valor desejado em um `std::atomic<float>` e
aplicar os novos coeficientes no início do próximo bloco, ou usar um
ramp. Para o djdeck, trocar na fronteira de bloco é suficiente.

### 5.5 Filtro

Um knob só, que varre de passa-baixa a passa-alta:

| Posição | Efeito |
|---|---|
| 0% | passa-alta, 20 kHz (in audível) |
| 50% | neutro (bypass) |
| 100% | passa-baixa, 20 Hz |

```cpp
// Padrão: knob bipolar com bypass no centro
filterSlider.setRange (-1.0f, 1.0f, 0.01f);
filterSlider.setValue (0.0f);
```

Implementação com os mesmos filtros IIR do EQ:

```cpp
void DeckPlayer::setFilterAmount (float amount)   // -1 .. +1
{
    if (amount >= 0.0f)
    {
        // passa-baixa: a frequência de corte desce conforme o knob sobe
        const auto cutoff = juce::jlimit (20.0f, 20000.0f,
                            juce::jmap (amount, 0.0f, 1.0f, 20000.0f, 20.0f));
        filter = juce::IIRCoefficients::makeLowPass (sampleRate, cutoff, 0.7f);
    }
    else
    {
        // passa-alta: a frequência de corte sobe conforme o knob desce
        const auto cutoff = juce::jlimit (20.0f, 20000.0f,
                            juce::jmap (amount, -1.0f, 0.0f, 20.0f, 20000.0f));
        filter = juce::IIRCoefficients::makeHighPass (sampleRate, cutoff, 0.7f);
    }
}
```

> **`juce::MapRange` não existe na JUCE.** O helper correto é
> `juce::jmap`, em `juce_core/maths/juce_MathsFunctions.h`:
> ```cpp
> template <typename Type>
> Type jmap (Type sourceValue, Type sourceRangeMin, Type sourceRangeMax,
>            Type targetRangeMin, Type targetRangeMax);
> ```
> E o clamp é `juce::jlimit(lowerLimit, upperLimit, value)` — **a ordem dos
> argumentos é (mínimo, máximo, valor)**, e o valor vem por último.

### 5.6 Fader de canal

O fader de volume do deck. No djdeck atual ele é o `volSlider` do `DeckGUI`,
com faixa 0.0–1.0. A especificação do Rekordbox usa um volume em dB com
-∞ no fundo:

```cpp
// -inf no fundo, 0 dB a 3/4 do curso
volumeFader.setRange (-60.0f, 6.0f, 0.1f);
volumeFader.setValue (0.0f);
```

Manter a faixa **linear 0–1** é mais simples e já funciona
(`DeckPlayer::setGain` / `getGain`). A conversão para dB pode vir depois,
sem quebrar a API — `setGain` já é o ponto de integração.

Desenho do fader (handle vertical com linha de posição, canal escuro):

```cpp
void ChannelFader::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().reduced (4);

    // Trilho
    g.setColour (RekordboxColours::canvas);
    g.fillRoundedRectangle (area.removeFromLeft (4).toFloat(), 2.0f);

    // Preenchimento até o handle
    const auto pos = (float) slider.getValue();
    const auto y = area.getBottom() - pos * area.getHeight();

    g.setColour (RekordboxColours::accent);
    g.fillRoundedRectangle (area.getX(), y, 4.0f, area.getBottom() - y, 2.0f);

    // Handle
    auto handle = area.withHeight (18).withY ((int) y - 9);
    g.setColour (RekordboxColours::textSecondary);
    g.fillRoundedRectangle (handle.toFloat(), 3.0f);
    g.setColour (RekordboxColours::canvas);
    g.fillRect (handle.getCentreX() - 1, handle.getY(), 2, handle.getHeight());
}
```

### 5.7 VU meter

#### Especificação

Um medidor de nível por canal, vertical, com:

- fundo escuro
- preenchimento em gradiente verde → amarelo → vermelho
- escala em dB
- LED de clip no topo

Mede o sinal **após** o fader de canal (pós-gain), que é o que o DJ
precisa ver: quanto está saindo.

#### Obter o nível

O nível é calculado na **thread de áudio**, dentro de
`DeckPlayer::getNextAudioBlock`, e publicado para a thread de GUI através
de um `juce::Atomic<float>`.

```cpp
// Em DeckPlayer — audio thread
void DeckPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock (bufferToFill);

    // Nível RMS do bloco, por canal
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
    {
        const auto magnitude = bufferToFill.buffer->getMagnitude (
            ch, bufferToFill.startSample, bufferToFill.buffer->getNumSamples());

        // Converte para dB
        const auto db = juce::Decibels::gainToDecibels (magnitude);
        levelDb[ch].set (db);
    }
}
```

> `juce::AudioBuffer<float>::getMagnitude(int channel, int startSample,
> int numSamples)` retorna a magnitude RMS do trecho — está em
> `juce_audio_basics/buffers/juce_AudioSampleBuffer.h`.
> `juce::Decibels::gainToDecibels<Type>(gain, mindB)`.

Publique o valor **já em dB** — a conversão RMS→dB é cara para repetir na
thread de GUI a 30 Hz para cada canal.

#### Suavização (ballistics)

Um VU meter real tem inércia: attack rápido, release lento. O `juce::dsp::
BallisticsFilter` faz exatamente isso.

```cpp
// No VUMeterComponent — GUI thread
void VUMeterComponent::timerCallback()
{
    const auto db = sourceLevelDb.get();       // Atomic<float>

    // Ballistics: attack 10 ms, release 300 ms
    juce::dsp::BallisticsFilter<float> ballistics;
    ballistics.setLevelCalculationType (juce::dsp::BallisticsFilterLevelCalculationType::peak);
    ballistics.setAttackTime (10.0f);
    ballistics.setReleaseTime (300.0f);
    // ...
}
```

O `BallisticsFilter` processa **amostra a amostra**, o que não serve para um
medidor atualizado a 30 Hz. Para uso em GUI, a abordagem prática é
aplicar o decay no próprio timer:

```cpp
void VUMeterComponent::timerCallback()
{
    const auto incoming = sourceLevelDb.get();

    // Decay exponencial entre atualizações: release de ~300 ms a 30 fps
    // (fator ~0.8 por frame para 60% de queda em 300 ms)
    currentLevelDb = juce::jmax (incoming, currentLevelDb * 0.8f);

    repaint();
}
```

> `juce::dsp::BallisticsFilter<SampleType>` está em
> `juce_dsp/processors/juce_BallisticsFilter.h`, com
> `setAttackTime(ms)`, `setReleaseTime(ms)` e
> `setLevelCalculationType(peak | RMS)`. Ele serve para o caso em que o
> medidor é processado na thread de áudio.

#### Desenho

```cpp
void VUMeterComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    // Fundo
    g.setColour (RekordboxColours::canvas);
    g.fillRect (area);

    const auto minDb = -60.0f;
    const auto maxDb =  0.0f;

    // Normaliza o nível para a altura, com o mínimo no fundo
    const auto normalised = juce::jlimit (0.0f, 1.0f,
                                (currentLevelDb - minDb) / (maxDb - minDb));
    const auto fillHeight = (int) (normalised * area.getHeight());

    if (fillHeight > 0)
    {
        // Gradiente verde -> amarelo -> vermelho, de baixo para cima
        juce::ColourGradient gradient (juce::Colours::green, 0.0f, (float) area.getBottom(),
                                       juce::Colours::red,   0.0f, (float) area.getY(), false);
        gradient.addColour (0.75, juce::Colours::yellow);

        g.setGradientFill (gradient);
        g.fillRect (area.getX(), area.getBottom() - fillHeight,
                    area.getWidth(), fillHeight);
    }

    // LED de clip
    if (clipDetected)
        g.setColour (RekordboxColours::cue);
    else
        g.setColour (RekordboxColours::canvas);
    g.fillRect (area.removeFromTop (4));

    drawScale (g, area);
}
```

#### Escala em dB

```cpp
void VUMeterComponent::drawScale (juce::Graphics& g,
                                  const juce::Rectangle<int>& area)
{
    const float marks[] = { 0.0f, -3.0f, -6.0f, -10.0f, -16.0f, -22.0f, -32.0f, -48.0f };
    const auto minDb = -60.0f;

    g.setFont (juce::Font (8.0f));
    g.setColour (RekordboxColours::textTertiary);

    for (const auto mark : marks)
    {
        const auto y = area.getBottom() - (mark / minDb) * area.getHeight();
        g.drawLine (area.getRight() - 4, (int) y, area.getRight(), (int) y, 1.0f);
        g.drawText (juce::String ((int) mark), area.getX(), (int) y - 6,
                    area.getWidth() - 6, 12, juce::Justification::topLeft, false);
    }
}
```

#### Thread-safety

O nível cruza a fronteira entre thread de áudio e thread de GUI. Use
`juce::Atomic<float>`:

```cpp
// No DeckPlayer
juce::Atomic<float> levelDb[2];

// No VUMeterComponent — recebe a referência
VUMeterComponent (juce::Atomic<float>& sourceToUse) : sourceLevelDb (sourceToUse) {}

// Leitura segura
const auto db = sourceLevelDb.get();
```

> `juce::Atomic<Type>` está em `juce_core/memory/juce_Atomic.h` e oferece
> `get()`, `set()`, `exchange()`.

### 5.8 CUE / PFL

Botão quadrado sob o fader. Não faz nada no djdeck atual — é headphone
monitoring (PFL = pre-fader listen).

```
┌─────┐
│ CUE │   ← quadrado, azul (#007DE1) quando ativo
└─────┘
```

```cpp
cueButton.setClickingTogglesState (true);
cueButton.setColour (juce::TextButton::buttonOnColourId, RekordboxColours::accent);
```

Implementação real: em PFL, o sinal do deck vai para a saída de fone em vez
da saída principal. Isso exige uma terceira saída de áudio ou roteamento por
`AudioDeviceManager`. Está fora do escopo do layout — a UI é o que importa
aqui.

### 5.9 MIXING / LEVEL

A faixa de monitoramento de fone, no rodapé do mixer. Dois knobs lado a lado,
com o ícone de fone à esquerda do MIXING.

```
┌───────────────────────────────────────────────┐
│  🎧   ( MIXING )   ( LEVEL )                   │
└───────────────────────────────────────────────┘
```

| Controle | Função | Faixa | Neutro |
|---|---|---|---|
| MIXING | mistura entre os dois decks no fone | 0 → 1 | centro = mistura igual |
| LEVEL | volume do fone | 0 → 1 | centro |

Ambos são knobs **unipolares** — o valor neutro é o centro do arco, mas o
caminho é só para um lado.

#### MIXING

Escolhe o que se ouve no fone: deck A, deck B ou a mistura dos dois. No
Rekordbox o knob gira entre os dois decks e o centro é a mistura.

```cpp
// Fração de deck B no fone: 0 = só deck A, 1 = só deck B, 0.5 = mistura
void MixerComponent::setHeadphoneMix (double bAmount)
{
    const auto b = juce::jlimit (0.0, 1.0, bAmount);

    // Ganhos constant-power, como no crossfader
    headphoneGainA = std::cos (b * juce::MathConstants<double>::halfPi);
    headphoneGainB = std::sin (b * juce::MathConstants<double>::halfPi);

    applyHeadphoneGains();
}
```

#### LEVEL

Volume geral do fone. Vai para a saída de fone, não para a mixagem principal.

```cpp
headphoneLevel.setRange (0.0f, 1.0f, 0.01f);
headphoneLevel.setValue (0.7f);
```

#### Implementação de áudio

O monitoramento de fone exige **uma segunda saída de áudio**. O
`juce::AudioDeviceManager` do djdeck (herdado de `juce::AudioAppComponent`)
configura uma única `AudioDeviceSetup` com um `outputDeviceName`.

Duas estratégias, em ordem de dificuldade:

**A) Dispositivo multicanal (recomendado).** Uma única interface com N
canais de saída — 2 estéreo para o master + 2 estéreo para o fone. O
`AudioDeviceManager` continua com um dispositivo só, mas o app roteia os
canais ele mesmo no callback de áudio.

```cpp
// No getNextAudioBlock do MainComponent
void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    // Canais 0-1: master (a mixagem normal)
    mixerSource.getNextAudioBlock (bufferToFill);

    // Canais 2-3: headphone (PFL) — depende do layout do dispositivo
    if (bufferToFill.buffer->getNumChannels() >= 4)
    {
        for (int ch = 2; ch < 4; ++ch)
        {
            auto* dest = bufferToFill.buffer->getWritePointer (ch);

            for (int i = 0; i < bufferToFill.buffer->getNumSamples(); ++i)
                dest[i] = 0.0f;

            // ... somar deckA * headphoneGainA + deckB * headphoneGainB
        }
    }
}
```

> **`AudioBuffer::getBuffer()` não existe na JUCE 9.** Para escrever num
> subrange de canais usa-se `getWritePointer(int channelNumber)` — que
> devolve o ponteiro do canal inteiro, ou
> `getWritePointer(int channelNumber, int sampleIndex)` para começar num
> sample específico. Também existem `clear()`, `clear(startSample, numSamples)`
> e `clear(channel, startSample, numSamples)`.
> `juce::AudioBuffer` está em
> `juce_audio_basics/buffers/juce_AudioSampleBuffer.h`.

O `AudioDeviceSetup` tem `outputChannels` (um `BigInteger` com os canais
ativos) — é ele que habilita os canais 2 e 3.

**B) Dispositivo de fone separado.** Requer uma segunda
`AudioDeviceSetup` e um segundo callback de áudio. O `AudioDeviceManager`
não suporta isso nativamente — é preciso um `juce::AudioDeviceManager`
adicional ou um callback próprio.

> `juce::AudioDeviceManager::AudioDeviceSetup` tem `outputDeviceName`,
> `inputDeviceName`, `sampleRate`, `bufferSize`, `inputChannels`,
> `outputChannels`, `useDefaultInputChannels` e `useDefaultOutputChannels`.
> `getAudioDeviceSetup()` e `setAudioDeviceSetup(const AudioDeviceSetup&,
> bool treatAsChosenDevice)` leem e escrevem a configuração.
> A classe está em `juce_audio_devices/audio_io/juce_AudioDeviceManager.h`.

**Estado no djdeck:** nada disso existe. O `MainComponent` atual usa
`setAudioChannels(0, 2)` — duas saídas estéreo, master apenas.

### 5.10 Crossfader

#### Especificação

Fader **horizontal** no fundo do mixer, entre os dois canais.

- Posição central = mistura equilibrada (0 dB em ambos)
- Todo à esquerda = deck A no máximo, deck B em -∞
- Todo à direita = deck B no máximo, deck A em -∞
- Botões de atribuição **1** e **2** acima do crossfader: a qual lado cada
  deck é atribuído

> Os botões 1/2 são **atribuição ao crossfader**, não seleção de deck. O deck
> ativo é outro conceito.

#### Onde aplicar o ganho

O `MixerAudioSource` não tem ganho por fonte, então o crossfader é aplicado
dentro de cada `DeckPlayer`, **multiplicando** o ganho do fader de canal:

```cpp
// No DeckPlayer
void DeckPlayer::setCrossfadeGain (float g)
{
    crossfadeGain = g;
    applyGain();
}

void DeckPlayer::applyGain()
{
    const auto linear = juce::Decibels::decibelsToGain (trimDb)
                      * juce::jlimit (0.0f, 1.0f, (float) channelGain)
                      * crossfadeGain;
    transportSource.setGain (linear);
}
```

#### Curva do crossfader

O crossfader não é linear no ganho — a percepção de volume não é linear
no ganho. O Rekordbox oferece curvas ("linear" e "constant power"). A
padrão em software DJ é **constant power**, que preserva a energia percebida
na saída:

```cpp
// Posição do crossfader: -1.0 (deck A) .. +1.0 (deck B)
// Retorna o par de ganhos (deckA, deckB) em escala linear

void CrossfaderComponent::computeGains (double position,
                                        float& gainA, float& gainB) const
{
    // Normaliza para 0..1
    const auto t = juce::jlimit (0.0, 1.0, (position + 1.0) * 0.5);

    // Constant power: as duas somam em potência constante
    gainA = std::cos (t * juce::MathConstants<double>::halfPi);
    gainB = std::sin (t * juce::MathConstants<double>::halfPi);
}
```

No centro (`t = 0.5`): `gainA = gainB = 0.707` — cada deck a -3 dB, com a
mesma energia total. Nas pontas: um em 1.0, o outro em 0.0.

#### Layout

```
        ┌─────────────────────────────────────────┐
        │  [1]  [2]        ← atribuição ao crossfader│
        │                                         │
        │  ──────────●────────────                │  ← fader horizontal
        │            A        B                   │
        └─────────────────────────────────────────┘
```

O botão **1** fica à esquerda, o **2** à direita, com o knob embaixo.

---

## 6. Elementos de interface

Todos os controles visuais são desenhados por um **LookAndFeel único**,
`RekordboxLookAndFeel`, derivado de `juce::LookAndFeel_V4`. O LookAndFeel é
aplicado uma única vez no `MainComponent` e vale para todos os filhos.

> **Ordem de declaração importa.** O LookAndFeel precisa ser declarado
> **antes** dos widgets que o usam no arquivo de cabeçalho — caso contrário
> é destruído antes deles e o app quebra ao fechar. No `MainComponent.h` o
> `lookAndFeel` já está declarado no fim dos members; ao introduzir novos
> componentes, mantenha-o como **último** member.

```cpp
// MainComponent.h — ordem correta
class MainComponent : public juce::AudioAppComponent
{
    ...
    juce::LookAndFeel_V4 lookAndFeel;   // por último
};
```

### 6.1 Knob

Knob rotativo com um **indicador**. Usado para TRIM, HIGH, MID, LOW, FILTER.

#### Aparência

```
      ┌───┐
    ╱       ╲        ← corpo circular escuro (#282828)
   │    │    │       ← indicador branco, apontando para o valor
    ╲       ╱
      └───┘
     TRIM              ← label abaixo, cinza
```

- Corpo: `panelRaised` (#282828), contorno `separator` (#323232)
- Indicador: linha branca de 2 px, do centro até 70% do raio
- Arco de valor: opcional, na cor de destaque

#### Implementação

Um `juce::Slider` com `RotaryVerticalDrag` e um `LookAndFeel` que desenha o
indicador.

```cpp
class KnobComponent : public juce::Slider
{
public:
    KnobComponent (juce::String labelToUse)
        : juce::Slider (juce::Slider::SliderStyle::RotaryVerticalDrag),
          caption (labelToUse)
    {
        setTextBoxStyle (juce::Slider::TextBoxBelow, false, 0, 0);
        setVelocityBasedMode (false);
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        auto labelArea = area.removeFromBottom (14);

        const auto radius = (float) juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 2.0f;
        const auto centre = area.getCentre().toFloat();
        const auto angle = getRotaryAngle();

        // Corpo
        g.setColour (RekordboxColours::panelRaised);
        g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2);

        g.setColour (RekordboxColours::separator);
        g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2, 1.0f);

        // Arco de valor
        {
            // O arco parte do centro (neutro) e cresce até o valor atual.
            // Num knob bipolar o neutro é o centro da faixa; num unipolar,
            // é o mínimo.
            const auto centreAngle = bipolar ? 0.0f : startAngle;
            const auto fromAngle = juce::jmin (centreAngle, angle);
            const auto toAngle   = juce::jmax (centreAngle, angle);

            if (toAngle - fromAngle > 0.001f)
            {
                juce::Path valueArc;
                valueArc.addCentredArc (centre.x, centre.y, radius - 1.0f, radius - 1.0f, 0.0f,
                                        fromAngle, toAngle, true);
                g.setColour (RekordboxColours::accent);
                g.strokePath (valueArc, juce::PathStrokeType (2.0f));
            }
        }

        // Indicador
        const auto indicatorLength = radius * 0.7f;
        const auto indicatorX = centre.x + std::sin (angle) * indicatorLength;
        const auto indicatorY = centre.y - std::cos (angle) * indicatorLength;

        g.setColour (RekordboxColours::textPrimary);
        g.drawLine (centre.x, centre.y, indicatorX, indicatorY, 2.0f);

        // Label
        g.setFont (juce::Font (11.0f));
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (caption, labelArea, juce::Justification::centred, true);
    }

private:
    juce::String caption;

    // Knobs bipolares (EQ, TRIM, FILTER) têm o neutro no centro da faixa:
    // o valor 0 ocupa o ângulo 0 (12h) e o arco cresce para os dois lados.
    bool bipolar = true;

    static constexpr float startAngle = -juce::MathConstants<float>::pi * 0.75f;
    static constexpr float endAngle   =  juce::MathConstants<float>::pi * 0.75f;

    float getRotaryAngle() const
    {
        // valueToProportionOfLength mapeia o valor da faixa para 0..1.
        // Para bipolar, o neutro (0) fica em 0.5 — que deve virar o ângulo 0.
        const auto proportion = (float) valueToProportionOfLength (getValue());

        if (bipolar)
            return (proportion * 2.0f - 1.0f) * (endAngle - startAngle) * 0.5f;

        return startAngle + proportion * (endAngle - startAngle);
    }
};
```

> **`getProportionOfValue()` não existe na JUCE.** A API correta é
> `Slider::valueToProportionOfLength(double value)`, que é pública e
> virtual (`juce_gui_basics/widgets/juce_Slider.h`, linha 830). Ela mapeia o
> valor atual da faixa para 0..1.
>
> Para um knob **bipolar**, o `valueToProportionOfLength` devolve 0.5 quando o
> valor é o centro da faixa — por isso o ângulo é recentrado por
> `(proportion * 2 - 1)`. Sem esse ajuste, o indicador apontaria para baixo
> no neutro.

> **Nota JUCE 9:** `juce::Graphics::drawArc` e `drawPie` **não existem** na
> JUCE 9.0.5 — foram removidos. Use `juce::Path::addCentredArc()` +
> `g.strokePath(path, PathStrokeType)`. As factories `addArc`,
> `addCentredArc` e `addPieSegment` estão em
> `juce_graphics/geometry/juce_Path.h`.
>
> `juce::PathStrokeType(float thickness, JointStyle jointStyle, EndCapStyle endStyle = butt)` —
> `JointStyle` tem `mitered`, `curved`, `beveled`; `EndCapStyle` tem `butt`,
> `square`, `rounded`. Está em `juce_graphics/geometry/juce_PathStrokeType.h`.

#### Ângulos

O Rekordbox usa um arco de aproximadamente **270°**, com a posição neutra
apontando para cima. Os knobs bipolares (EQ, TRIM, FILTER) têm o centro
como neutro.

```cpp
// Slider rotativo com arco customizado
slider.setRotaryParameters (startAngleRadians, endAngleRadians, stopAtEnd);
```

> `juce::Slider::RotaryParameters` tem `startAngleRadians`, `endAngleRadians`
> e `stopAtEnd`. Os ângulos são em radianos, **sentido horário a partir do
> topo**. `setRotaryParameters(float, float, bool)` é o atalho.

### 6.2 Slider

Fader linear — usado no canal de volume (vertical) e no crossfader
(horizontal).

#### Vertical (fader de canal)

- Trilho: 4 px de largura, cor `canvas`
- Preenchimento: `accent`, da posição atual até o topo
- Handle: retângulo arredondado de 18 px de altura, `textSecondary`, com
  uma linha central escura

#### Horizontal (crossfader)

- Trilho: 4 px de altura
- Handle: losango ou cápsula, 24 px de largura

Implementação com `juce::Slider` + `LookAndFeel`:

```cpp
class RekordboxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        const auto isVertical = style == juce::Slider::LinearVertical;
        const auto centre = juce::Point<float> (x + width * 0.5f, y + height * 0.5f);

        if (isVertical)
        {
            // Trilho
            g.setColour (RekordboxColours::canvas);
            g.fillRoundedRectangle (centre.x - 2.0f, (float) y, 4.0f, (float) height, 2.0f);

            // Preenchimento
            const auto handleY = y + height * (1.0f - sliderPos);
            g.setColour (RekordboxColours::accent);
            g.fillRoundedRectangle (centre.x - 2.0f, handleY, 4.0f,
                                    (float) (y + height) - handleY, 2.0f);

            // Handle
            auto handle = juce::Rectangle<float> (centre.x - 12.0f, handleY - 9.0f, 24.0f, 18.0f);
            g.setColour (RekordboxColours::textSecondary);
            g.fillRoundedRectangle (handle, 3.0f);
            g.setColour (RekordboxColours::canvas);
            g.fillRect (handle.getCentreX() - 1.0f, handle.getY(), 2.0f, handle.getHeight());
        }
        else
        {
            // Versão horizontal, espelhada
            const auto handleX = x + width * sliderPos;
            g.setColour (RekordboxColours::canvas);
            g.fillRoundedRectangle ((float) x, centre.y - 2.0f, (float) width, 4.0f, 2.0f);

            g.setColour (RekordboxColours::accent);
            g.fillRoundedRectangle ((float) x, centre.y - 2.0f, handleX - x, 4.0f, 2.0f);

            auto handle = juce::Rectangle<float> (handleX - 9.0f, centre.y - 12.0f, 18.0f, 24.0f);
            g.setColour (RekordboxColours::textSecondary);
            g.fillRoundedRectangle (handle, 3.0f);
            g.setColour (RekordboxColours::canvas);
            g.fillRect (handle.getX(), handle.getCentreY() - 1.0f, handle.getWidth(), 2.0f);
        }
    }
};
```

> A assinatura exata de `LookAndFeel_V4::drawLinearSlider` em JUCE 9.0.5:
> ```cpp
> void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
>                        float sliderPos, float minSliderPos, float maxSliderPos,
>                        juce::Slider::SliderStyle, juce::Slider&) override;
> ```

### 6.3 Jog wheel

O disco de manipulação. É o controle mais identificável de um software DJ e
o mais difícil de acertar.

#### Especificação

```
        ┌───────────────┐
     ╱     140.00        ╲      ← BPM no centro, 28 px, branco
    │      ±16%           │     ← pitch %, 13 px, cinza
    │                     │
     ╲                   ╱
        └───────────────┘
```

- Disco com ~140 px de diâmetro
- BPM no centro
- Pitch % abaixo do BPM
- Marcador de faixa do pitch (±8%, ±16%, ±50%) em volta da borda
- CUE button ao lado (esquerda no deck 1, direita no deck 2)
- Play/Pause abaixo do CUE

#### Comportamento

O jog wheel tem **dois modos**, alternados pelo clique no disco:

| Modo | Arrasto | Efeito |
|---|---|---|
| **Touch** (padrão) | circular | faz "scratch" — move a posição da áudio |
| **Pitch** | vertical | ajusta a velocidade de reprodução |

No modo touch, o ângulo de arrasto do mouse mapeia para deslocamento de
posição. É o inverso do que parece natural: girar o disco **para a direita**
avança a áudio.

#### Implementação

O jog wheel **não** é um `juce::Slider` — o `Slider` não tem o conceito de
rotação livre com inércia. É um `Component` customizado que captura os
eventos de mouse.

```cpp
class JogWheelComponent : public juce::Component
{
public:
    explicit JogWheelComponent (bool mirroredToUse)
        : mirrored (mirroredToUse)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        const auto radius = (float) juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
        const auto centre = area.getCentre().toFloat();

        // Disco
        g.setColour (RekordboxColours::panelRaised);
        g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2);

        g.setColour (RekordboxColours::separator);
        g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2, 1.0f);

        // BPM
        g.setFont (juce::Font (28.0f));
        g.setColour (RekordboxColours::textPrimary);
        g.drawText (bpmText, area.removeFromTop ((int) radius), juce::Justification::centred, true);

        // Pitch %
        g.setFont (juce::Font (13.0f));
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (pitchText, area, juce::Justification::centredTop, true);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        lastAngle = getAngleAt (e.position);
        dragging = true;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! dragging)
            return;

        const auto angle = getAngleAt (e.position);

        // Diferença com wrap em ±pi
        auto delta = angle - lastAngle;
        if (delta > juce::MathConstants<float>::pi)  delta -= 2.0f * juce::MathConstants<float>::pi;
        if (delta < -juce::MathConstants<float>::pi) delta += 2.0f * juce::MathConstants<float>::pi;

        lastAngle = angle;

        // Modo pitch: arrasto vertical
        if (mode == Mode::pitch)
        {
            pitchChange = juce::jlimit (-1.0f, 1.0f, pitchChange - e.getDistanceFromDragStartY() * 0.005f);
            return;
        }

        // Modo touch: arrasto circular vira deslocamento de posição.
        // Girar para a direita (delta > 0) AVANÇA a áudio.
        if (onScratch)
            onScratch (delta * 0.15f);   // ~0.15 s por radiano
    }

    void mouseUp (const juce::MouseEvent&) override { dragging = false; }

private:
    bool mirrored;
    bool dragging = false;
    float lastAngle = 0.0f;
    float pitchChange = 0.0f;

    juce::String bpmText = "--.--";
    juce::String pitchText = "0.00%";

    enum class Mode { touch, pitch };
    Mode mode = Mode::touch;

    std::function<void (float)> onScratch;

    // Ângulo do ponteiro em torno do centro, em radianos, 0 = topo
    float getAngleAt (juce::Point<float> position) const
    {
        auto area = getLocalBounds().toFloat();
        const auto centre = area.getCentre();
        const auto delta = position - centre;

        auto angle = std::atan2 (delta.x, -delta.y);   // 0 = topo, horário
        return angle;
    }
};
```

> `juce::Path::addCentredArc` usa **0 = topo, sentido horário** — o mesmo
> sistema do `getAngleAt` acima, o que evita conversões.

**O `mirrored` só afeta a posição dos botões laterais** (CUE e play/pause ao
lado do disco), não o sentido da rotação.

#### Marcadores de faixa do pitch

A borda do disco mostra a faixa de pitch disponível. O segmento atual é
destacado em vermelho:

```cpp
// Segmento ativo (vermelho) na borda do disco
juce::Path pitchSegment;
pitchSegment.addCentredArc (centre.x, centre.y, radius - 1.0f, radius - 1.0f, 0.0f,
                            startAngle, endAngle, true);
g.setColour (RekordboxColours::cue);
g.strokePath (pitchSegment, juce::PathStrokeType (3.0f));
```

### 6.4 Botões

Botões retangulares com cantos levemente arredondados. Estados:

| Estado | Fundo | Texto |
|---|---|---|
| Normal | `panelRaised` (#282828) | `textSecondary` |
| Hover | `hover` (#2A2A2A) | `textPrimary` |
| Pressionado | `accent` (#007DE1) | `textPrimary` |
| Toggle on | `accent` (#007DE1) | `textPrimary` |
| Desabilitado | `panel` | `textDisabled` |

```cpp
class RekordboxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto area = button.getLocalBounds().toFloat().reduced (1.0f);
        const auto cornerSize = 3.0f;

        juce::Colour fill = backgroundColour;
        if (shouldDrawButtonAsDown)
            fill = RekordboxColours::accent;
        else if (shouldDrawButtonAsHighlighted)
            fill = RekordboxColours::hover;

        g.setColour (fill);
        g.fillRoundedRectangle (area, cornerSize);

        g.setColour (RekordboxColours::separator);
        g.drawRoundedRectangle (area, cornerSize, 1.0f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override
    {
        g.setFont (getTextButtonFont (button, button.getHeight()));

        auto colour = button.findColour (button.getToggleState()
                                            ? juce::TextButton::textColourOnId
                                            : juce::TextButton::textColourOffId);
        if (! button.isEnabled())
            colour = colour.withMultipliedAlpha (0.5f);

        g.setColour (colour);
        g.drawFittedText (button.getButtonText(), 2, 2,
                          button.getWidth() - 4, button.getHeight() - 4,
                          juce::Justification::centred, 2);
    }
};
```

> `LookAndFeel_V4::drawButtonBackground` em JUCE 9.0.5:
> ```cpp
> void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
>                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
> ```
> `drawButtonText(juce::Graphics&, juce::TextButton&, bool isMouseOverButton, bool isButtonDown)`
> vem de `juce::Button::LookAndFeelMethods`.

### 6.5 Pads (Hot Cue / Beat Jump)

Grade de 8 pads, 4×2, no deck. As cores de `RekordboxColours::hotCue[]`.

```
  ┌────┬────┐
  │ A  │ E  │
  ├────┼────┤
  │ B  │ F  │
  ├────┼────┤
  │ C  │ G  │
  ├────┼────┤
  │ D  │ H  │
  └────┴────┘
```

Cada pad mostra a **letra** e, quando definido, o **timestamp** da posição
de cue.

```cpp
class HotCuePadsComponent : public juce::Component
{
public:
    HotCuePadsComponent()
    {
        for (int i = 0; i < 8; ++i)
        {
            padButtons[i] = std::make_unique<juce::TextButton> (juce::String ((char) ('A' + i)));
            padButtons[i]->setClickingTogglesState (true);
            padButtons[i]->addListener (this);
            addAndMakeVisible (*padButtons[i]);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        const auto padW = area.getWidth() / 4;
        const auto padH = area.getHeight() / 2;

        for (int i = 0; i < 8; ++i)
        {
            const auto row = i / 4;
            const auto col = i % 4;

            auto r = area.removeFromTop (padH).removeFromLeft (padW);
            padButtons[i]->setBounds (r.reduced (2));
        }
    }

private:
    std::unique_ptr<juce::TextButton> padButtons[8];
};
```

Para colorir cada pad individualmente, defina a cor **no próprio botão** —
o LookAndFeel global não distingue um pad de outro:

```cpp
padButtons[i]->setColour (juce::TextButton::buttonOnColourId,
                          RekordboxColours::hotCue[i]);
```

O **Beat Jump** tem a mesma estrutura de grade, mas os rótulos são
fracões de batida (1/8, 1/4, 1/2, 1, 2, 4, 8) em vez de letras.

---

## 7. Deck

### 7.1 Cabeçalho do deck

```
┌────────────────────────────────────────────────────────────┐
│ (1)  Título da Faixa                          [SYNC] [MASTER]│
│      Artista                                     140.00  Gm │
│                                          -05:10.5  00:00.4 │
└────────────────────────────────────────────────────────────┘
```

| Elemento | Posição | Estilo |
|---|---|---|
| Número do deck | topo esquerdo | círculo, 18 px, branco |
| Título | após o número | 16 px, branco |
| Artista | abaixo do título | 13 px, cinza |
| BPM + key | topo direito | 16 px, branco; key em badge vermelho |
| Tempo decorrido | abaixo | 11 px, cinza |
| Tempo restante | abaixo | 11 px, cinza |
| SYNC | topo direito, coluna | botão retangular; azul quando ativo |
| MASTER | ao lado do SYNC | laranja quando ativo |

### 7.2 Layout do deck

O deck 1 e o deck 2 são o **mesmo componente**, com `mirrored` invertido.
Dentro do deck:

```
┌──────────────────────────────────────────┐
│  DeckHeaderComponent                     │  ← info da faixa
├──────────────────────────────────────────┤
│  MiniWaveformComponent                   │  ← onda estática, agulha móvel
├────────────────────┬─────────────────────┤
│                    │                     │
│  BeatJumpComponent │  JogWheelComponent  │  ← deck 1: beat jump à esquerda
│  OU                │      + CUE          │
│  HotCuePadsComponent│     + Play         │  ← deck 2: hot cue à direita
│                    │                     │
├────────────────────┴─────────────────────┤
│  Footer: atribuição 1-4, MIXING, LEVEL   │
└──────────────────────────────────────────┘
```

### 7.3 Espelhamento

O `mirrored` controla:

| Elemento | `mirrored = false` (deck 1) | `mirrored = true` (deck 2) |
|---|---|---|
| Grade de pads | à esquerda | à direita |
| Jog wheel | à direita | à esquerda |
| CUE / Play | à esquerda do jog | à direita do jog |
| Rótulos de EQ | à direita | à esquerda |

### 7.4 Estado atual

O `DeckGUI` atual (`Source/DeckGUI.cpp`) é muito mais simples:

```cpp
void DeckGUI::resized()
{
    auto area = getLocalBounds();
    int h = area.getHeight();
    int row = h / 14;

    // 4 botões de transporte na mesma faixa
    // 3 sliders (vol, speed) — o terceiro fica vazio
    // 1 slider de posição
    // waveform ocupa o resto
}
```

Faltam: cabeçalho com BPM/key, jog wheel, pads, EQ, mixer, crossfader,
sync/master, hot cues, loops.

---

## 8. Biblioteca

### 8.1 Estrutura

```
LibraryComponent
├── LibraryTreeView        (árvore, ~30% da largura)
│   └── LibraryTreeModel
└── TrackListView          (lista, ~70% da largura)
    ├── Header: título + contagem + busca + view modes
    └── TrackListModel
```

### 8.2 Árvore de navegação

O `juce::TreeView` da JUCE 9 usa `juce::TreeViewItem` — a classe está em
`juce_gui_basics/widgets/juce_TreeView.h` (não em arquivo separado), e
`setRootItem()` recebe um `TreeViewItem*`.

```cpp
class LibraryTreeModel : public juce::TreeViewItem
{
public:
    LibraryTreeModel (const juce::String& name) : title (name) {}

    void paintItem (juce::Graphics& g, int width, int height) override
    {
        // Desenha o ícone + o texto
    }

    void itemClicked (const juce::MouseEvent&) override { }
    void itemDoubleClicked (const juce::MouseEvent&) override { }

    juce::String title;
};

void LibraryComponent::buildTree()
{
    auto root = std::make_unique<LibraryTreeModel> ("Collection");
    root->addSubItem (std::make_unique<LibraryTreeModel> ("Playlists"));
    root->addSubItem (std::make_unique<LibraryTreeModel> ("Explorer"));

    treeView.setRootItem (root.get());
    treeRoot = std::move (root);   // o TreeView não toma posse
}
```

> `juce::TreeViewItem` tem `addSubItem(TreeViewItem*, int insertPosition = -1)`,
> `paintItem(Graphics&, int width, int height)`, `itemClicked(...)`,
> `itemDoubleClicked(...)` e `createItemComponent()`. **O `TreeView` não tem
> posse do root** — guarde-o como member.
>
> O método de desenho chama-se **`paintItem`**, não `paintTreeItem`. Se
> quiser desenhar o botão de expandir, há
> `paintOpenCloseButton(Graphics&, const Rectangle<float>&, Colour, bool)`.

Nós esperados, espelhando o Rekordbox:

| Nó | ícone | expansível |
|---|---|---|
| Collection | nota musical | não |
| Playlists | lista | sim |
| Explorer | pasta | sim |
| Devices | — | não |
| Histories | — | não |
| Recordings | — | não |

### 8.3 Tabela de faixas

O `juce::TableListBox` com um `TableListBoxModel`. O modelo atual do djdeck
(`Source/PlaylistComponent.cpp`) já implementa o padrão correto, com 3
colunas: Título, Artista, Duração.

As colunas são declaradas no **header** do `TableListBox`, não no modelo:

```cpp
// No construtor do componente
playlistTable.getHeader().addColumn ("Título", columnTitle, 200);
playlistTable.getHeader().addColumn ("Artista", columnArtist, 150);
playlistTable.getHeader().addColumn ("Duração", columnLength, 100);
```

> `juce::TableListBox::getHeader()` retorna um
> `juce::TableHeaderComponent&`, que tem
> `addColumn(const String& columnName, int columnId, int width)` e
> `setColumnWidth(int columnId, int newWidth)`.

Colunas alvo, da referência:

| Coluna | Conteúdo |
|---|---|
| # | número + marcador de cue |
| Preview | miniatura da waveform, colorida, com hot cues A–H |
| Title | título branco, artista cinza |
| Artist | |
| BPM | "140.00" + key "Gm" |
| Album | |
| Genre | |
| Rating | 5 estrelas, preenchidas em âmbar `#DEAB38` |
| Time | duração |

```cpp
void TrackListModel::paintCell (juce::Graphics& g, int rowNumber, int columnId,
                                int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= tracks.size())
        return;

    const auto& track = tracks[rowNumber];

    switch (columnId)
    {
        case columnTitle:
            // Título branco, artista cinza
            break;

        case columnRating:
        {
            // 5 estrelas
            for (int i = 0; i < 5; ++i)
            {
                g.setColour (i < track.rating ? RekordboxColours::rating
                                              : RekordboxColours::textDisabled);
                // desenho da estrela
            }
            break;
        }
    }
}
```

### 8.4 Mini waveform na coluna Preview

A prévia é a mesma waveform da big waveform, reduzida. O `AudioThumbnail`
desenha em qualquer resolução — basta dar a ele a área pequena da coluna:

```cpp
void TrackPreviewComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour (RekordboxColours::canvas);
    g.fillRect (area);

    if (thumbnail.getTotalLength() > 0.0)
    {
        g.setColour (RekordboxColours::waveMid);
        thumbnail.drawChannel (g, area, 0.0, thumbnail.getTotalLength(), 0, 1.0f);
    }

    // Marcadores de hot cue A-H, na cor de cada pad
    for (int i = 0; i < hotCueTimes.size(); ++i)
    {
        const auto x = area.getX() + (int) ((hotCueTimes[i] / thumbnail.getTotalLength())
                                            * area.getWidth());
        g.setColour (RekordboxColours::hotCue[i]);
        g.fillRect (x, area.getY(), 2, area.getHeight());
    }
}
```

### 8.5 Divisor redimensionável

**`juce::SplitterComponent` não existe na JUCE 9.** O mecanismo equivalente
é o `juce::StretchableLayoutManager` com uma `StretchableLayoutResizerBar`.

```cpp
class LibraryComponent : public juce::Component
{
private:
    juce::StretchableLayoutManager layoutManager;

    void resized() override
    {
        layoutManager.setItemLayout (0, 150, 600, 300);   // árvore
        layoutManager.setItemLayout (1, 200, -0.5, -0.7); // lista

        Component* comps[] = { &treeView, &trackList };
        layoutManager.layOutComponents (comps, 2,
                                       0, 0, getWidth(), getHeight(),
                                       false,  // vertically = false → lado a lado
                                       true);  // preenche a altura toda
    }
};
```

> `juce::StretchableLayoutManager::setItemLayout(int itemIndex, double minimumSize,
> double maximumSize, double preferredSize)` — números **negativos** são
> proporções do espaço disponível.
>
> `layOutComponents` tem **dois parâmetros `bool` no final**, além dos
> inteiros:
> ```cpp
> void layOutComponents (Component** components, int numComponents,
>                        int x, int y, int width, int height,
>                        bool vertically, bool resizeOtherDimension);
> ```
> - `vertically = false` → os componentes são distribuídos **ao longo do eixo
>   X**, cada um recebendo a sua **largura**. É o caso da biblioteca, onde
>   árvore e lista ficam lado a lado.
> - `vertically = true` → distribute ao longo do **eixo Y**, cada um
>   recebendo a sua **altura**. É o caso de uma pilha vertical de painéis.
>
> Com `resizeOtherDimension = true`, os componentes também preenchem a
> dimensão perpendicular (a árvore ocupa a altura toda).
>
> `juce::StretchableLayoutResizerBar` fica em
> `juce_gui_basics/layout/juce_StretchableLayoutResizerBar.h`.

---

## 9. Arquitetura de componentes

### 9.1 Regras

1. **Um componente por arquivo.** Nome do arquivo = nome da classe.
2. **Sem estado global.** Cada componente recebe o que precisa no
   construtor.
3. **LookAndFeel único**, aplicado no `MainComponent`.
4. **Componentes de deck espelhados** via parâmetro `bool mirrored`, não via
   subclasses duplicadas.
5. **DSP dentro do `DeckPlayer`.** Os componentes de UI não tocam em áudio —
   eles chamam métodos do player (`setTrimDb`, `setEqGain`, `setCrossfadeGain`).

### 9.2 Divisão de responsabilidade

```
DeckGUI / DeckComponent  ──UI──▶  DeckPlayer  ──DSP──▶  áudio
       │                              ▲
       └── botões, sliders, pads      │
                                      └── setTrimDb / setEq / setFilter / setCrossfade
```

O `DeckPlayer` é a **única** classe que toca no áudio. A UI só chama
setters. Isso mantém o DSP testável isoladamente e evita que a thread de GUI
escreva em coeficientes de filtro em uso.

### 9.3 Onde cada coisa vive

| Classe | Responsabilidade |
|---|---|
| `MainComponent` | layout raiz, LookAndFeel, ponte com o áudio |
| `DeckPlayer` | áudio do deck: transporte, DSP, ganho |
| `DeckComponent` | UI do deck (header, pads, jog) |
| `MixerComponent` | UI do mixer (strips, crossfader) |
| `ChannelStripComponent` | UI de um canal |
| `BigWaveformComponent` | a onda rolante de topo |
| `LibraryComponent` | biblioteca + árvore |
| `RekordboxLookAndFeel` | desenho de todos os widgets |
| `RekordboxColours` | paleta |

---

## 10. Migração da API JUCE 9: armadilhas conhecidas

Verificadas no código-fonte da JUCE 9.0.5. Estas são APIs que **existiam em
versões anteriores** ou que têm nome parecido — compilam errado ou não
compilam.

| API antiga / inexistente | O que usar em JUCE 9.0.5 |
|---|---|
| `Graphics::drawArc(...)` | `Path::addCentredArc(...)` + `g.strokePath(path, PathStrokeType)` |
| `Graphics::drawPie(...)` | `Path::addPieSegment(...)` + `g.fillPath(path)` |
| `Slider::getProportionOfValue()` | `Slider::valueToProportionOfLength(getValue())` |
| `juce::MapRange<float>` | `juce::jmap(val, srcMin, srcMax, dstMin, dstMax)` |
| `juce::IIR::Filter` (audio_basics) | `juce::IIRFilter` |
| `juce::IIR::Coefficients` (audio_basics) | `juce::IIRCoefficients` |
| `juce::SplitterComponent` | `juce::StretchableLayoutManager` + `StretchableLayoutResizerBar` |
| `Grid::setCol(n, ...)` / `setRow(n, ...)` | `grid.templateColumns` / `grid.templateRows` (arrays de `TrackInfo`) + `GridItem::setArea(...)` |
| `dsp::FFT::getMagnitude()` / `getPhase()` | `performRealOnlyForwardTransform()` / `performFrequencyOnlyForwardTransform()` |
| `setDoubleClickReturnValue(val, bool)` | `setDoubleClickReturnValue(bool enabled, double value)` — **bool primeiro** |

### 10.1 Mudanças de módulo

| Classe | Módulo em JUCE 9 |
|---|---|
| `juce::AudioProcessor` | `juce_audio_processors_headless` |
| `juce::AudioTransportSource` | `juce_audio_devices` (era `audio_basics`) |
| `juce::AudioThumbnail` | `juce_audio_utils` |

O `CMakeLists.txt` do djdeck já linka `juce::juce_audio_utils`, que depende
de `juce_audio_processors` → `juce_audio_processors_headless`. Portanto
`juce::AudioProcessor` já está disponível.

### 10.2 `juce::Grid` — a API mudou

Em JUCE 9 o Grid segue o modelo do CSS Grid. Não existem `setCol`/`setRow`:

```cpp
// Definição das tracks (colunas/linhas)
juce::Grid grid;

grid.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Fr (4)),   // deck A
                         juce::Grid::TrackInfo (juce::Grid::Fr (1)),   // mixer
                         juce::Grid::TrackInfo (juce::Grid::Fr (4)) }; // deck B

grid.setGap (juce::Grid::Px (2));

// Itens posicionados
grid.items.add (juce::GridItem (deckA).withArea (juce::GridItem::Property (0),
                                                 juce::GridItem::Property (0)));
grid.items.add (juce::GridItem (mixer).withArea (juce::GridItem::Property (0),
                                                 juce::GridItem::Property (1)));
grid.items.add (juce::GridItem (deckB).withArea (juce::GridItem::Property (0),
                                                 juce::GridItem::Property (2)));

// Layout
grid.performLayout (getLocalBounds());
```

> `juce::Grid::TrackInfo` aceita `Px` ou `Fr`. `juce::Grid::Px` e
> `juce::Grid::Fr` são structs distintas — `Fr` só aceita inteiros.
> `juce::GridItem::Property` aceita `int`, `String`, `Span` ou
> `Keyword::autoValue`. `withArea(Property rowStart, Property columnStart)`
> ocupa 1×1; a sobrecarga de 4 argumentos define o span.
> `juce::Grid` está em `juce_gui_basics/layout/juce_Grid.h` e
> `juce::GridItem` em `juce_gui_basics/layout/juce_GridItem.h`.

### 10.3 FFT — para a onda em 3 camadas (fase futura)

A onda em 3 camadas (low/mid/high) exige análise espectral. Em JUCE 9 o
`juce::dsp::FFT` **não tem** `getMagnitude()`/`getPhase()`:

```cpp
// JUCE 9 — só existem estas
void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept;
void performRealOnlyForwardTransform (float* inputOutputData,
                                      bool onlyCalculateNonNegativeFrequencies = false) const noexcept;
void performFrequencyOnlyForwardTransform (float* inputOutputData,
                                           bool onlyCalculateNonNegativeFrequencies = false) const noexcept;
int getSize() const noexcept;
```

O tipo complexo é `juce::dsp::Complex<float>` (alias de `std::complex<float>`).

Fluxo para obter as três bandas:

```cpp
// 1) Janela (Hann) para reduzir leakage
juce::dsp::WindowingFunction<float> windowing (fftSize,
                            juce::dsp::WindowingFunction<float>::hann);
windowing.multiplyWithWindowingTable (timeDomain, fftSize);

// 2) FFT
juce::dsp::FFT fft (fftOrder);          // tamanho = 2^order
fft.performRealOnlyForwardTransform (timeDomain, true);

// 3) Magnitudes — o resultado JÁ É a magnitude
//    (performFrequencyOnlyForwardTransform faz isso direto)
```

> `juce::dsp::WindowingFunction<FloatType>` tem
> `WindowingMethod { rectangular, triangular, hann, hamming, blackman,
> blackmanHarris, flatTop, kaiser }` e
> `multiplyWithWindowingTable(FloatType*, size_t)`.
> `juce::dsp::FFT` está em `juce_dsp/frequency/juce_FFT.h`; a
> `WindowingFunction` em `juce_dsp/frequency/juce_Windowing.h`.

**Ordem de execução:** esta seção documenta a API para quando a onda em 3
camadas for implantada. A primeira fase usa a big waveform padrão (min/max),
que não precisa de FFT.

### 10.4 Formato `.dat` — pré-computar picos

Para zoom fluido e para a prévia na biblioteca, o padrão é pré-computar os
picos min/max fora da thread de áudio. O formato `.dat` do
[audiowaveform](https://github.com/bbc/audiowaveform) é a referência padrão
da indústria:

```
Header (v2, 24 bytes, little-endian)
  int32_t   version = 2
  uint32_t  flags        (bit 0: 1 = 8-bit, 0 = 16-bit)
  int32_t   sample_rate
  int32_t   samples_per_pixel
  uint32_t  length       (número de pares min/max por canal)
  int32_t   channels

Dados: pares (min, max) interleaved por canal
  16-bit: int16_t, faixa -32768..+32767
  8-bit:  int8_t,  faixa -128..+127
```

Para o djdeck, um formato próprio simplificado em memória é suficiente — o
`.dat` importa se a compatibilidade com arquivos do Rekordbox for objetivo.

---

## 11. Estado atual vs. alvo

Verificado no commit `4203707` (tag `v0.1.0`).

|| Área | Estado | Observação |
|---|---|---|---|
| Layout raiz | ✅ completo | 4 seções: header (~10%), big waveforms (~22%), decks+mixer (~39%), biblioteca (~28%) |
| Big waveform | ✅ presente | `BigWaveformComponent` — agulha fixa no centro, onda rolante, janela simétrica de 30 s |
| Waveform do deck | ✅ presente | `MiniWaveformComponent` — faixa estática, agulha móvel esquerda → direita |
| Carregamento da waveform | ✅ OK | Usa `FileInputSource` (não carrega arquivo inteiro em memória) |
| Mixer (UI) | ✅ presente | `MixerComponent` + `ChannelStripComponent` × 2 |
| TRIM | ✅ presente | Knob bipolar ±12 dB |
| EQ 3 bandas | ✅ presente | Low shelf 250 Hz, peak 1 kHz, high shelf 4 kHz — ±12 dB |
| Filtro | ✅ presente | Passa-baixa / passa-alta bipolar |
| Fader de canal | ✅ presente | `volumeFader`, faixa linear 0–1 |
| VU meter | ✅ presente | `VUMeterComponent` — decay em domínio linear, escala dB, LED de clip |
| Crossfader | ✅ presente | `CrossfaderComponent` — curva constant power, aplicado no `DeckPlayer` |
| MIXING / LEVEL (fone) | ✅ presente | `mixingFader` + `levelFader` no `MixerComponent` (segunda saída de áudio ainda não) |
| Knob | ✅ presente | `KnobComponent` — rotativo, bipolar/unipolar, arco de valor |
| Jog wheel | ✅ presente | `JogWheelComponent` — modo touch (scratch) e pitch (vertical), rotação do disco |
| Pads (hot cue / beat jump) | ✅ presente | `HotCuePadsComponent` + `BeatJumpComponent` |
| Cabeçalho do deck | ✅ presente | `DeckHeaderComponent` — título, artista, BPM, key, tempo decorrido/restante |
| LookAndFeel | ✅ customizado | `RekordboxLookAndFeel` — desenho de knobs, sliders, botões, VU meter |
| Biblioteca em árvore | ✅ presente | `LibraryComponent` + `LibraryTreeModel` + `StretchableLayoutManager` |
| Preview com waveform | ❌ ausente | Coluna Preview na biblioteca não tem miniatura de onda |
| Árvore de navegação | ✅ presente | Collection, Playlists, Explorer, Devices, Histories, Recordings |
| Fonte customizada | ❌ ausente | Fonte do sistema |
| Cor customizada | ✅ presente | `RekordboxColours` — paleta completa medida no Rekordbox |
| Metadados (TagLib) | ✅ presente | `TagLibReader` + `TagLibFileStream` — ID3, Vorbis, MP4, APE, caminhos largos no Windows |
| Detecção de BPM | ❌ ausente | TagLib lê a tag `BPM` quando o arquivo já tem; não há detecção de áudio |

### 11.1 Ordem sugerida de implementação (próximas fases)

1. **Detecção de BPM** — avaliar `libebguess` ou `aubio`; o TagLib só lê o BPM quando a tag já existe.
2. **Preview com waveform na biblioteca** — coluna Preview na tabela de faixas.
3. **Persistência de hot cues** — gravar posição dos cues no arquivo (tag ANLZ do Rekordbox ou similar).
4. **Caminhos Unicode no Windows** — validar `TagLibFileStream` com caminhos acentuados.
5. **Segunda saída de áudio (fone)** — necessário para PFL/MIXING/LEVEL reais (§5.9).
6. **Fonte customizada** — carregar via `BinaryData` (ex.: XXIIAven).
7. **DVS / timecode** — fora do escopo atual (decisão no ROADMAP.md).

---

## 12. Referências

### Documentação

- [JUCE — Tutorial: Customise the look and feel of your app](https://juce.com/tutorials/tutorial_look_and_feel_customisation/)
  — a base para `RekordboxLookAndFeel`
- [JUCE — Tutorial: Audio thumbnails](https://juce.com/tutorials/tutorial_audio_thumbnail/)
  — o mecanismo da waveform
- [Fórum JUCE — Custom GUI, Knobs, Buttons and Sliders](https://forum.juce.com/t/custom-gui-knobs-buttons-and-sliders/55449)
  — discussão sobre knobs customizados
- [BBC — audiowaveform: Data Format](https://github.com/bbc/audiowaveform/blob/master/doc/DataFormat.md)
  — formato `.dat` de picos pré-computados

### Código

- `jcurtis4207/Juce-Plugins` (MIT) — referência de VU meter e knobs.
  Adaptado em `Scratch/Meters.h` e `Scratch/GUI-Components.h`.
  Atenção: o código é de 2021, baseado em APIs da JUCE 6 — os exemplos de
  `drawArc` **não compilam** na JUCE 9.

### Dentro do projeto

- `Scratch/Meters.h` — `MeterBand`, `LevelMeter`, `ClipLight`
- `Scratch/GUI-Components.h` — `SmallKnob`, `OuterKnob`, `BigKnob`,
  `VerticalSlider`
- `docs/ROADMAP.md` — decisões de arquitetura e pendências





