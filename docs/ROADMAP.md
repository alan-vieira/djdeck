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
- [ ] **Biblioteca para metadados** → **TagLib** (decidido).
      Vai substituir o preenchimento manual de título/artista/duração.
- [ ] **Listar metadados na playlist** — colunas de título, artista, álbum e
      BPM.
- [ ] **Biblioteca para BPM** — ainda não definida. Candidatos:
      [libebguess](https://github.com/break/libebguess) (porta do GNU
      SoundTouch) e [aubio](https://aub.io). Precisa avaliar licença e
      precisão antes de adotar.

---

## Fora do escopo

- Timecode / vinyl control (decisão explícita — ver acima).
