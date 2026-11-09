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

    deckGUI1 = std::make_unique<DeckGUI>(1, deckPlayer1.get(),
                                         formatManager,
                                         thumbnailCache);

    deckGUI2 = std::make_unique<DeckGUI>(2, deckPlayer2.get(),
                                         formatManager,
                                         thumbnailCache);

    playlist = std::make_unique<PlaylistComponent>();

    // Duplo-clique numa faixa da playlist carrega no Deck 1.
    // O arrraste da faixa até um deck funciona por drag-and-drop nativo do
    // JUCE: a PlaylistComponent fornece o payload (getDragSourceDescription)
    // e o DeckGUI recebe o drop (itemDropped).
    playlist->onSongSelected = [this] (const juce::URL& url)
    {
        if (deckGUI1 != nullptr)
            deckGUI1->loadFile (url);
    };

    addAndMakeVisible(*deckGUI1);
    addAndMakeVisible(*deckGUI2);
    addAndMakeVisible(*playlist);
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
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Botão settings no topo
    settingsButton.setBounds(area.removeFromTop(40).removeFromRight(150));

    // Playlist à esquerda (30% da largura)
    if (playlist != nullptr)
    {
        auto playlistWidth = static_cast<int>(area.getWidth() * 0.30f);
        playlist->setBounds(area.removeFromLeft(playlistWidth));
    }

    // Área restante para os 2 decks lado a lado
    if (deckGUI1 != nullptr && deckGUI2 != nullptr)
    {
        auto decksArea = area;
        int halfW = decksArea.getWidth() / 2;
        deckGUI1->setBounds(decksArea.removeFromLeft(halfW));
        deckGUI2->setBounds(decksArea);
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
