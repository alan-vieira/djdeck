# Roadmap

Planejamento do projeto. A ordem dos itens reflete a prioridade atual.

> **Guia visual e de construção:** ver
> [`DOCUMENTACAO_LAYOUT_UI.md`](DOCUMENTACAO_LAYOUT_UI.md) — paleta de cores
> medida, divisão de layout, especificação dos componentes (big waveform,
> mixer, EQ, VU meter, jog wheel, knobs, biblioteca) e as armadilhas da API
> JUCE 9.

---

## Decisões de arquitetura

| Decisão | Motivo |
|---|---|
| **TagLib** para extração de metadados | Biblioteca C++ open source e multiplataforma, com suporte a tags ID3 (MP3), Vorbis Comments (OGG/FLAC), MP4 e APE. Substitui a leitura manual de metadados. |
| **Não usar Xwax** | Xwax é escrito em C e focado no controle via *timecode* (vinyl control). Como o projeto **não vai trabalhar com timecode**, não há razão para trazer essa dependência. |
| **Sem DVS por enquanto** | O Digital Vinyl System exige timecode. Fica para uma fase futura, caso o controle por vinyl seja realmente desejado. |

### Alternativas avaliadas para DVS / vinyl control

| Opção | Situação |
|---|---|
| **Ms Pinky** | Não suporta M1 (Protocolo de Vinil Subtrativo). |
| **Xwax** | Escrito em C. Descartado — o projeto não vai usar timecode. |
| **Implementação própria** | Em aberto. Só faz sentido se o vinyl control entrar no escopo. |

---

## Geral

- [ ] **Esquema de cores e gráficos de UI customizados** — hoje a interface usa
  os componentes padrão do JUCE com um `LookAndFeel_V4`.
  O layout seguirá a **temática do Rekordbox**: biblioteca de faixas à
  esquerda, decks dispostos lado a lado à direita, tema escuro e alto
  contraste entre a waveform e o fundo.
- [ ] **Fonte customizada** (open source) — hoje usa a fonte padrão do sistema.
- [ ] **Lógica de interpolação** (pitch e afinação) — hoje o pitch é alterado
  por *resampling* simples, o que desloca a duração. O objetivo é manter a
  duração constante com interpolação de qualidade.
- [ ] **Digital Vinyl System (DVS)** — ver [Alternativas avaliadas](#alternativas-avaliadas-para-dvs--vinyl-control).
  Bloqueado por depender de timecode.

---

## Mixer

- [ ] **Filtro** — passa-baixa / passa-alta.
- [ ] **Equalizador de 3 bandas** (low / mid / high).
- [ ] **Controle de crossfade** entre os decks.
- [ ] **Gain por player** — o ganho já existe por deck (slider de 0.0 a 1.0);
      falta implementar o balanceamento de crossfade entre eles.

---

## Display do playhead

- [x] Atualizar a posição da faixa — o playhead já se move em tempo real
      (timer de ~30 fps no `DeckGUI`).
- [ ] **Feedback visual de amplitude ou “power spectrum”** — hoje a waveform é
  estática; o objetivo é exibir o espectro/amplitude em tempo real durante a
  reprodução.

---

## Área de “Playlist”

- [x] **Arrastar “track” para o player** — implementado na v0.1.0 com
      drag-and-drop nativo do JUCE e payload tipado.
- [x] **Biblioteca para metadados** → **TagLib** (decidido).
      TagLib v2.3.2 integrado via `FetchContent`, com build estático e o
      mesmo runtime `/MT` do app. Le e escreve tags ID3 (MP3), Vorbis
      Comments (OGG/FLAC/Opus), MP4 (M4A) e APE.
- [x] **Persistir a biblioteca** — gravada em XML no diretório de dados da
      aplicação (`%APPDATA%/djdeck/library.xml`), com escrita atômica
      (arquivo temporário + rename). Faixas cujo arquivo não existe mais são
      ignoradas na carga; os metadados são relidos do disco.

#### Como os projetos de referência persistem

Comparação com `TheAudioProgrammer/juceDjApp` e `dubstep-warrior/DJDecks`
(ambos JUCE C++), verificados no código:

| | juceDjApp | DJDecks | djdeck |
|---|---|---|---|
| Formato | XML (`XmlElement`) | XML (`ValueTree` → `writeToStream`) | XML (`XmlElement`) |
| Local | `~/Music/TAP DJ App/Playlist.xml` | caminho **hardcoded** `C:/Otodecks/...` | `%APPDATA%/djdeck/library.xml` |
| Grava quando | ao adicionar faixa | só no destructor | ao adicionar **e** no destructor |
| Metadados | TagLib (`TagReader`) | — | TagLib (`TagLibReader`) |
| Estrutura | `TABLE_DATA` → `HEADERS`/`COLUMN` + `LIBRARY`/`AUDIOFILE` | `ValueTree("main")` → pastas → faixas | `djdeckLibrary` → `track` |

Três lições aplicadas:
1. **XML** é o formato que os dois usam — mantém o djdeck alinhado.
2. **`writeTo()`** no lugar de `writeToFile()` (depreciada no JUCE 9).
3. **Gravar ao adicionar, não só no destructor** — o DJDecks perde a lista
   montada se o app cair; o djdeck grava nos dois momentos.

O `juce::ValueTree` do DJDecks é uma abstração de árvore mais elegante que o
`XmlElement` cru. Fica como melhoria futura se a biblioteca ganhar playlists
e folders aninhados.

- [x] **Listar metadados na playlist** — colunas de título, artista, BPM,
      key, álbum, gênero, rating (5 estrelas) e duração.
- [ ] **Caminhos Unicode no Windows** — o `FileStream` do próprio TagLib
      usa `fopen(const char*)`, que depende da codepage ANSI. O djdeck usa um
      adaptador (`TagLibFileStream`) que abre por caminho largo (`_wfopen`).
      **Pendente:** validar com uma faixa em caminho acentuado.
- [ ] **Hot cue persistente** — os pads já guardam a posição em memória;
      gravar no arquivo depende de definir em qual tag (o Rekordbox usa o
      banco `ANLZ`, não tags de texto).
- [ ] **Biblioteca para BPM** — ainda não definida. Candidatos:
      [libebguess](https://github.com/break/libebguess) (porta do GNU
      SoundTouch) e [aubio](https://aub.io). Precisa avaliar licença e
      precisão antes de adotar.
      *Nota:* o TagLib lê o BPM quando o arquivo já o tem na tag `BPM`
      (o Rekordbox grava), mas não o detecta.

---

## Fora do escopo

- Timecode / vinyl control (decisão explícita — ver acima).
