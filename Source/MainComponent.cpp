#include "MainComponent.h"
#include "SettingsDialog.h"

//==============================================================================
MainComponent::MainComponent()
    : deviceScanner (deviceManager)   // usa o AudioDeviceManager herdado de AudioAppComponent
{
    formatManager.registerBasicFormats();

    // Cria os componentes filhos ANTES de setAudioChannels(), porque o callback de
    // áudio (prepareToPlay) pode ser disparado durante a inicialização do device.
    deckPlayer1 = std::make_unique<DeckPlayer>(formatManager);
    deckPlayer2 = std::make_unique<DeckPlayer>(formatManager);

    // Deck 1: Beat Jump à esquerda, jog wheel à direita (não espelhado).
    // Deck 2: Hot Cue à direita, jog wheel à esquerda (espelhado).
    deck1 = std::make_unique<DeckComponent>(1, false, false,
                                             deckPlayer1.get(),
                                             formatManager,
                                             thumbnailCache);

    deck2 = std::make_unique<DeckComponent>(2, true, true,
                                             deckPlayer2.get(),
                                             formatManager,
                                             thumbnailCache);

    // Seções do layout raiz.
    globalHeader = std::make_unique<GlobalHeaderComponent>();
    bigWaveforms = std::make_unique<BigWaveformsComponent>(formatManager, thumbnailCache,
                                                            deckPlayer1.get(), deckPlayer2.get());
    mixer = std::make_unique<MixerComponent>(deckPlayer1.get(), deckPlayer2.get());
    library = std::make_unique<LibraryComponent>();

    // Duplo-clique numa faixa da lista carrega no Deck 1.
    library->getTrackList().onSongSelected = [this] (const juce::URL& url, double bpm,
                                                     const juce::String& title,
                                                     const juce::String& artist, int targetDeckId)
    {
        if (targetDeckId == 1 && deck1 != nullptr)
        {
            deck1->loadFile (url, bpm, title, artist);
            if (bigWaveforms != nullptr)
                bigWaveforms->loadURL (1, url);
        }
        else if (targetDeckId == 2 && deck2 != nullptr)
        {
            deck2->loadFile (url, bpm, title, artist);
            if (bigWaveforms != nullptr)
                bigWaveforms->loadURL (2, url);
        }
    };

    addAndMakeVisible(*globalHeader);
    addAndMakeVisible(*bigWaveforms);
    addAndMakeVisible(*deck1);
    addAndMakeVisible(*mixer);
    addAndMakeVisible(*deck2);
    addAndMakeVisible(*library);
    addAndMakeVisible(settingsButton);

    settingsButton.addListener(this);
    settingsButton.setTooltip("Configurações de dispositivo de áudio");

    setLookAndFeel(&lookAndFeel);

    // IMPORTANTE: setSize() precisa vir DEPOIS de addAndMakeVisible().
    // setSize() dispara resized(), e é resized() que distribui os bounds entre
    // os filhos. Se chamarmos setSize() antes dos filhos existirem, resized()
    // roda com a lista de filhos vazia e eles ficam com tamanho 0x0 -> tela preta.
    // Como a janela depois define bounds idênticos (1600x900), resized() não
    // dispara novamente, e os filhos nunca receberiam bounds.
    setSize (1600, 900);

    // Só inicializa o áudio depois que tudo acima existe.
    setAudioChannels(0, 2);

    deviceManager.addChangeListener(&deviceScanner);
}

MainComponent::~MainComponent()
{
    settingsButton.removeListener(this);
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
    g.fillAll(RekordboxColours::background);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // ---- Seção 1: header (~10%) ----
    {
        const auto headerHeight = juce::jlimit (40, 90, area.getHeight() / 10);

        if (globalHeader != nullptr)
            globalHeader->setBounds (area.removeFromTop (headerHeight));
    }

    // ---- Seção 2: big waveforms (~22%) ----
    {
        const auto waveformHeight = juce::jlimit (80, 220, area.getHeight() / 5);

        if (bigWaveforms != nullptr)
            bigWaveforms->setBounds (area.removeFromTop (waveformHeight));
    }

    // ---- Seção 3: decks + mixer (~39%) ----
    {
        const auto decksHeight = juce::jlimit (200, 400,
                                  static_cast<int> (area.getHeight() * 0.39));

        auto decksArea = area.removeFromTop (decksHeight);

        // Colunas: deck A ~45%, mixer ~10%, deck B ~45%.
        if (deck1 != nullptr && mixer != nullptr && deck2 != nullptr)
        {
            const auto totalWidth = decksArea.getWidth();
            const auto mixerWidth = juce::jlimit (140, 260, totalWidth / 10);
            const auto deckWidth = (totalWidth - mixerWidth) / 2;

            // Posicoes explicitas: deck A | mixer | deck B.
            deck1->setBounds (decksArea.getX(), decksArea.getY(), deckWidth, decksArea.getHeight());

            mixer->setBounds (decksArea.getX() + deckWidth, decksArea.getY(),
                              mixerWidth, decksArea.getHeight());

            deck2->setBounds (decksArea.getX() + deckWidth + mixerWidth, decksArea.getY(),
                              totalWidth - deckWidth - mixerWidth, decksArea.getHeight());
        }
    }

    // ---- Seção 4: biblioteca (~28%) ----
    {
        if (library != nullptr)
            library->setBounds (area);
    }

    // Botão de settings, sobreposto ao header.
    if (globalHeader != nullptr)
    {
        auto headerBounds = globalHeader->getBounds();
        settingsButton.setBounds (headerBounds.removeFromRight (110).reduced (4, 8));
    }
}

//==============================================================================
void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &settingsButton)
    {
        // O diálogo é mantido como member para não vazar memória. É destruído
        // junto com o MainComponent.
        if (settingsDialog == nullptr)
            settingsDialog = std::make_unique<SettingsDialog>(deviceManager);

        settingsDialog->setVisible(true);
    }
}
