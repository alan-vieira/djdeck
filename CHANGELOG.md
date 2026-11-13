# Changelog

Todas as mudanças relevantes deste projeto são documentadas aqui.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) e
o versionamento segue [SemVer](https://semver.org/lang/pt-BR/).

---

## [Não publicado]

Nada ainda.

---

## [0.3.0] — 2026-11-13

Versão estável com mixer completo, equalizador de 3 bandas, filtro, crossfader,
big/mini waveform sincronizadas, carregamento de waveform via `FileInputSource`
e testes automatizados.

### Adicionado

- **Testes automatizados** com Catch2 v3.5.2:
  - `tests/main.cpp` — ponto de entrada que inicializa o JUCE dentro de `main()`
    (compatível com app de console).
  - `tests/test_song.cpp` — `Song::getFormattedLength`, `getDisplayName`,
    `applyMetadata`.
  - `tests/test_deckplayer.cpp` — `setGain/getGain`, `setChannelGain`,
    `setTrimDb`, `setCrossfadeGain/getCrossfadeGain`, `setCue/isCue`,
    `setSpeed/getSpeed`, `setPositionRelative/getPositionRelative`,
    `applyGain`.
  - `tests/test_crossfader.cpp` — `CrossfaderComponent::computeGains`
    (curva constant power).
  - `tests/test_waveform.cpp` — registro de MP3 no `AudioFormatManager`,
    `createReaderFor` de MP3 real, `AudioThumbnail` com `FileInputSource`,
    caches separados (mini/big) sem corrupção, recarregamento e troca de
    arquivo.
- **`.gitignore`** — ignora artefatos de build, scratch e logs
  (`Builds/`, `BuildsTests/`, `Scratch/`, `*.log`, etc.).
- **Carregamento de waveform via `FileInputSource`** — o `AudioThumbnail`
  abre o arquivo por demanda, sem carregar o áudio inteiro em memória.
- **Caches de thumbnail separados:** `miniThumbnailCache` e
  `bigThumbnailCache` em `MainComponent` — evitam corrupção entre a mini e a
  big waveform do mesmo arquivo.
- **Sincronização deck ↔ waveform** via callback `onTrackLoaded` em
  `DeckComponent`, evitando includes circulares.

### Corrigido

- **Buffer de waveform** — `BigWaveformComponent` agora rastreia estado
  `fileLoaded` e repinta após o carregamento; não há mais leitura parcial ou
  silenciosa de falhas.
- **Big/mini waveform sincronizadas** — cada uma usa seu próprio
  `AudioThumbnailCache`, evitando que o carregamento de um sobrescreva o
  outro.
- **Inicialização JUCE para testes de console** — `tests/main.cpp` inicializa
  o JUCE dentro de `main()`; `Catch2::Catch2` (sem main) é usado no target de
  testes, não `Catch2::Catch2WithMain`.
- **Versão** — atualizada para `0.3.0` em `CMakeLists.txt` e
  `Source/Main.cpp` (`ProjectInfo::versionString`), consistente com a tag
  `v0.3.0`.

### Alterado

- `CMakeLists.txt` — target de testes usa `Catch2::Catch2` (sem main) e inclui
  `tests/test_waveform.cpp`; `Source/Song.cpp`, `Source/DeckPlayer.cpp` e
  `Source/CrossfaderComponent.cpp` são linkados nos testes.
- `Source/Main.cpp` — `ProjectInfo::versionString = "0.3.0"`.
- `Source/LibraryStore.cpp` — versão da biblioteca XML atualizada para `2`.

---

## [0.2.0] — 2026-11-09

Segundo release estável. Inclui as correções de build do JUCE 9, suporte a
drag-and-drop de faixas, mixer de dois canais, configuração de dispositivo de
áudio e persistência da biblioteca em XML.

### Adicionado

- **Dois decks** com controles de transporte (`Play`, `Stop`, `<<`, `>>`),
  gain, velocidade (0.5×–3.0×) e seek.
- **Mixer** de dois canais summed (`juce::MixerAudioSource`).
- **Playlist** em `TableListBox` com colunas de título, artista e duração.
- **Arrastar faixa da playlist para um deck** — drag-and-drop com payload
  tipado (`PlaylistDragPayload`).
- **Duplo-clique** na playlist carrega no Deck 1.
- **Waveform** com `AudioThumbnail` compartilhado entre os decks.
- **Diálogo de configurações de áudio** (device, sample rate, buffer size).
- **Detecção de troca de dispositivo de áudio** via `AudioDeviceScanner`.
- **Runtime estático do MSVC** (`/MT`).
- **JUCE 9.0.5** via `FetchContent`.
- Documentação inicial: README, CHANGELOG, LICENSE, `docs/ROADMAP.md`.

### Corrigido

- Build quebrado por `MainMinimal.cpp` (símbolo removido no JUCE 9).
- Tela preta por `setSize()` chamado antes de `addAndMakeVisible()`.
- Crash de device manager (declaração duplicada em `MainComponent`).
- Herança inválida de `DragAndDropContainer`.
- Use-after-free no `AudioThumbnail` e no `thumbnailCache`.
- Double-free no `SettingsDialog`.
- Ponteiro para temporário (`getUnchecked()` → `getReference()`).
- Layout dos botões de transporte e controles sem clique.
- Vazamento de diálogo e carregamento duplo no drag.

### Removido

- Caixas de diagnóstico de arranque e protótipos movidos para `Scratch/`.

---

## [0.1.0] — 2026-11-09

Primeira versão funcional. A aplicação **compila sem erros e sem warnings**,
abre com a interface completa e o fluxo principal (importar → arrastar para um
deck → reproduzir) está verificado em execução.

### Adicionado

- **Dois decks** com controles de transporte (`Play`, `Stop`, `<<`, `>>`),
  gain, velocidade (0.5×–3.0×) e seek.
- **Mixer** de dois canais summed (`juce::MixerAudioSource`) alimentado pelos
  decks e ligado à saída do dispositivo de áudio.
- **Playlist** em `TableListBox` com colunas de título, artista e duração.
  - Importação via botão *Adicionar faixa* (seletor multi-arquivo).
  - Importação via arrastar arquivo do Explorer para a playlist.
- **Arrastar faixa da playlist para um deck** — drag-and-drop interno com
  payload tipado (`PlaylistDragPayload`), e destaque visual do deck sob o
  cursor durante o arraste.
- **Duplo-clique** numa linha da playlist carrega a faixa no Deck 1.
- **Waveform** com `AudioThumbnail` compartilhado entre os decks, desenhada em
  laranja com playhead branco.
- **Diálogo de configurações de áudio** com seletor de dispositivo, sample
  rate e tamanho de buffer.
- **Detecção de troca de dispositivo de áudio** via `AudioDeviceScanner`.
- **Runtime estático do MSVC** (`/MT`), dispensando DLLs redistribuíveis.
- **JUCE 9.0.5** obtido por `FetchContent` — não requer instalação manual.
- Documentação inicial: README, CHANGELOG, LICENSE e `docs/ROADMAP.md` com as
  decisões de arquitetura.

### Corrigido

Bugs que impediam a aplicação de compilar ou de funcionar corretamente:

- **Build quebrado:** `Source/MainMinimal.cpp` usava
  `juce::NativeMessageBox::showMessageBox()`, removida no JUCE 9 (o símbolo só
  existe sob `JUCE_MODAL_LOOPS_PERMITTED`, que é `0` no Windows), e definia um
  segundo `START_JUCE_APPLICATION` causando `main()` duplicado no link.
  Arquivo movido para `Scratch/` e excluído do glob do CMake.
- **Tela preta:** `setSize()` era chamado antes de `addAndMakeVisible()`,
  disparando `resized()` sem filhos — todos ficavam com bounds `0×0` e, como a
  janela depois definia bounds idênticos, `resized()` não re-disparava.
- **Crash de device manager:** `MainComponent` declarava o próprio
  `deviceManager`, escondendo o `protected AudioDeviceManager&` herdado de
  `AudioAppComponent`. Passavam a existir dois managers distintos, e o
  `SettingsDialog` controlava um device já destruído.
- **Herança inválida:** `MainComponent` herdava de `juce::DragAndDropContainer`
  sem ser um `Component`. A herança foi restaurada de forma correta
  (mixin convivendo com `AudioAppComponent`), o que habilitou o drag-and-drop.
- **Use-after-free no `AudioThumbnail`:** o `AudioBuffer` era uma variável
  local, mas `AudioThumbnail::setSource()` não copia os dados — mantém o
  ponteiro e o lê numa thread de background. O buffer passou a ser member.
- **Use-after-free no shutdown:** `thumbnailCache` era destruído antes dos
  decks, e o `AudioThumbnail` de cada deck o acessa no destructor.
- **Double-free:** `SettingsDialog` passava um `AudioDeviceSelectorComponent`
  alocado como member por valor ao `setContentOwned()`, que também o deleta.
  Passou a ser alocado no heap.
- **Ponteiro para temporário:** `&songs.getUnchecked(...)` no JUCE 9 retorna o
  endereço de uma cópia temporária (`operator[]` e `getUnchecked()` retornam
  por valor). Substituído por `getReference()`.
- **Propriedade do reader de áudio:** `createReaderFor()` devolvia um ponteiro
  cru sem dono; o reader agora é devolvido ao `AudioFormatReaderSource`, e a
  taxa de amostragem é capturada antes de soltar o ponteiro.
- **Layout dos botões de transporte:** `removeFromTop()` encadeado devolvia
  retângulos novos a cada chamada, empilhando os quatro botões no mesmo
  lugar. Agora o mesmo retângulo é dividido em quatro partes.
- **Controles sem clique:** `setInterceptsMouseClicks(false, false)` desligava
  o clique dos filhos; corrigido para `(true, true)`.
- **Vazamento de diálogo:** `dialog.release()` após `setVisible(true)`
  transferia a posse sem liberar a memória. O diálogo passou a ser member
  (`std::unique_ptr`).
- **Carregamento duplo no drag:** a seleção da linha carregava a faixa no
  Deck 1 *e* o drop carregava no Deck 2. A seleção agora apenas marca a linha.
- **Drop ignorado silenciosamente:** com `setInterceptsMouseClicks(false, …)`,
  o `hitTest()` do `DeckGUI` falhava nas áreas sem filho e o
  `getComponentAt()` devolvia `nullptr`, descartando o drop.

### Removido

- Caixas de diálogo de diagnóstico (`NativeMessageBox::showMessageBoxAsync`)
  que apareciam no arranque da aplicação.
- Arquivos de diagnóstico e protótipos (`MainMinimal.cpp`, `MainWindowTest.cpp.bak`,
  `AudioTestComponent.*`, `MinimalMainComponent.*`), movidos para `Scratch/` e
  mantidos fora do build.

### Decisões de arquitetura

- **TagLib** será a biblioteca de metadados (título, artista, álbum, BPM),
  substituindo o preenchimento manual atual.
- **Xwax não será usado.** O projeto não vai trabalhar com timecode, então o
  DVS / vinyl control fica fora do escopo por ora.

### Conhecido

- **Acentuação quebrada nos textos da interface** (`DuraÃ§Ãqo`, `TÃtulo`).
  Causa identificada: `juce::String(const char*)` no JUCE 9 usa
  `CharPointer_ASCII` e não decodifica UTF-8. Requer
  `juce::String::fromUTF8()` — correção pendente.
- A coluna **duração** exibe sempre `0:00` e o **artista** sempre
  *“Artista desconhecido”* — os metadados ainda não são extraídos dos arquivos.
- A playlist não é persistida entre execuções.
- Não há crossfader, equalização nem hotcues (adicionados na v0.3.0).

[0.3.0]: https://github.com/alan-vieira/djdeck/releases/tag/v0.3.0
[0.2.0]: https://github.com/alan-vieira/djdeck/releases/tag/v0.2.0
[0.1.0]: https://github.com/alan-vieira/djdeck/releases/tag/v0.1.0
