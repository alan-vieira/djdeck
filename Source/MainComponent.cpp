#include "MainComponent.h"
#include "SettingsDialog.h"
#include "LayoutConfig.hpp"

//==============================================================================
MainComponent::MainComponent()
    : deviceScanner (deviceManager)   // usa o AudioDeviceManager herdado de AudioAppComponent
{
    juce::Logger::writeToLog ("[DIAG] MainComponent constructor start");
    formatManager.registerBasicFormats();

    // Cria os componentes filhos ANTES de setAudioChannels(), porque o callback de
    // áudio (prepareToPlay) pode ser disparado durante a inicialização do device.
    deckPlayer1 = std::make_unique<DeckPlayer>(formatManager);
    deckPlayer2 = std::make_unique<DeckPlayer>(formatManager);

    // Deck 1: Beat Jump à esquerda, jog wheel à direita (não espelhado).
    // Deck 2: Hot Cue à direita, jog wheel à esquerda (espelhado).
    //
    // Cada deck usa o cache da sua resolucao: o AudioThumbnailCache e
    // indexado pelo hash do ARQUIVO, nao pela resolucao. Compartilhar o
    // cache entre Mini (1024) e Big (2048) do mesmo arquivo faria os dois
    // lerem dados de resolucao errada do mesmo slot.
    deck1 = std::make_unique<DeckComponent>(1, false, false,
                                             deckPlayer1.get(),
                                             formatManager,
                                             miniThumbnailCache);
    juce::Logger::writeToLog ("[DIAG] deck1 created");

    deck2 = std::make_unique<DeckComponent>(2, true, true,
                                             deckPlayer2.get(),
                                             formatManager,
                                             miniThumbnailCache);
    juce::Logger::writeToLog ("[DIAG] deck2 created");

    // Seções do layout raiz.
    bigWaveforms = std::make_unique<BigWaveformsComponent>(formatManager, bigThumbnailCache,
                                                           deckPlayer1.get(), deckPlayer2.get());
    mixer = std::make_unique<MixerComponent>(deckPlayer1.get(), deckPlayer2.get());
    library = std::make_unique<LibraryComponent> (formatManager);

    // Sincroniza a big waveform do topo com o deck que carregou a faixa.
    // Passa por um callback e nao por cast para o pai: o DeckComponent nao
    // conhece o MainComponent (evita include circular).
    auto syncBigWaveform = [this] (int deckId, const juce::URL& url)
    {
        if (bigWaveforms != nullptr)
            bigWaveforms->loadURL (deckId, url);
    };

    deck1->onTrackLoaded = syncBigWaveform;
    deck2->onTrackLoaded = syncBigWaveform;

    // Duplo-clique numa faixa da lista carrega no Deck 1.
    //
    // NOTA: a big waveform e notificada pelo callback onTrackLoaded do deck,
    // nao aqui — assim o duplo-clique e o arrastar-soltar seguem o MESMO
    // caminho e nao ha risco de a big waveform ficar dessincronizada.
    library->getTrackList().onSongSelected = [this] (const juce::URL& url, double bpm,
                                                     const juce::String& title,
                                                     const juce::String& artist,
                                                     const juce::String& key,
                                                     int targetDeckId)
    {
        if (targetDeckId == 1 && deck1 != nullptr)
        {
            deck1->loadFile (url, bpm, title, artist, key);
        }
        else if (targetDeckId == 2 && deck2 != nullptr)
        {
            deck2->loadFile (url, bpm, title, artist, key);
        }
    };

    addAndMakeVisible(*bigWaveforms);
    addAndMakeVisible(*mixer);
    addAndMakeVisible(*deck1);
    addAndMakeVisible(*deck2);
    addAndMakeVisible(*library);

    setLookAndFeel(&lookAndFeel);

    // IMPORTANTE: setSize() precisa vir DEPOIS de addAndMakeVisible().
    // setSize() dispara resized(), e é resized() que distribui os bounds entre
    // os filhos. Se chamarmos setSize() antes dos filhos existirem, resized()
    // roda com a lista de filhos vazia e eles ficam com tamanho 0x0 -> tela preta.
    // Como a janela depois define bounds idênticos (1452x813), resized() não
    // dispara novamente, e os filhos nunca receberiam bounds.
    setSize (djdeck::layout::screenWidth, djdeck::layout::screenHeight);
    juce::Logger::writeToLog ("[DIAG] setSize done");

    // Só inicializa o áudio depois que tudo acima existe.
    setAudioChannels(0, 2);
    juce::Logger::writeToLog ("[DIAG] setAudioChannels done");

    deviceManager.addChangeListener(&deviceScanner);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
    deviceManager.removeChangeListener(&deviceScanner);
    setLookAndFeel(nullptr);
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Conecta os decks ao mixer uma única vez. addInputSource() ignora entradas
    // repetidas, mas registrar aqui evita depender da ordem de chamada.
    mixerSource.addInputSource(deckPlayer1.get(), false);
    mixerSource.addInputSource(deckPlayer2.get(), false);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    // mixerSource.releaseResources() já propaga para os decks registrados,
    // então não é necessário liberá-los individualmente aqui.
    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(djdeck::layout::colours::Background);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // A tela JUCE e o canvas do Figma: ambos usam 1452x813.
    // O offset centraliza qualquer sobra caso as dimensoes sejam alteradas.
    const int offsetX = (djdeck::layout::screenWidth  - djdeck::layout::figmaCanvas::widthPx)  / 2;
    const int offsetY = (djdeck::layout::screenHeight - djdeck::layout::figmaCanvas::heightPx) / 2;

    // Big waveforms: alturas dos nós do Figma (bigwaveform1 + bigwaveform2)
    const auto waveformsHeight =
        djdeck::layout::figmaNodebigwaveform1::heightPx + djdeck::layout::figmaNodebigwaveform2::heightPx;

    if (bigWaveforms != nullptr)
        bigWaveforms->setBounds (area.removeFromTop (waveformsHeight));

    // deck1 | mixer | deck2 — posições e tamanhos exatos do Figma (fonte: layout.json / figmaNodes)
    if (deck1 != nullptr && mixer != nullptr && deck2 != nullptr)
    {
        deck1->setBounds (djdeck::layout::figmaNodedeck1::xPx + offsetX,
                          djdeck::layout::figmaNodedeck1::yPx + offsetY,
                          djdeck::layout::figmaNodedeck1::widthPx,
                          djdeck::layout::figmaNodedeck1::heightPx);

        mixer->setBounds (djdeck::layout::figmaNodemixer::xPx + offsetX,
                          djdeck::layout::figmaNodemixer::yPx + offsetY,
                          djdeck::layout::figmaNodemixer::widthPx,
                          djdeck::layout::figmaNodemixer::heightPx);

        deck2->setBounds (djdeck::layout::figmaNodedeck2::xPx + offsetX,
                          djdeck::layout::figmaNodedeck2::yPx + offsetY,
                          djdeck::layout::figmaNodedeck2::widthPx,
                          djdeck::layout::figmaNodedeck2::heightPx);
    }

    // biblioteca — nó do Figma preenche o restante abaixo dos decks
    if (library != nullptr)
        library->setBounds (djdeck::layout::figmaNodebiblioteca::xPx + offsetX,
                            djdeck::layout::figmaNodebiblioteca::yPx + offsetY,
                            djdeck::layout::figmaNodebiblioteca::widthPx,
                            djdeck::layout::figmaNodebiblioteca::heightPx);
}

//==============================================================================
void MainComponent::buttonClicked(juce::Button* button)
{
    juce::ignoreUnused(button);
}
