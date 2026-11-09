# Changelog

Todas as mudanças relevantes deste projeto são documentadas aqui.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/) e
o versionamento segue [SemVer](https://semver.org/lang/pt-BR/).

---

## [Não publicado]

Nada ainda.

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
- Não há crossfader, equalização nem hotcues.

[0.1.0]: https://github.com/SEU-USUARIO/djdeck/releases/tag/v0.1.0
