#include "DeckComponent.h"
#include "RekordboxColours.h"

//==============================================================================
DeckComponent::DeckComponent (int deckId,
                              bool mirroredToUse,
                              bool useHotCuePadsToUse,
                              DeckPlayer* playerToUse,
                              juce::AudioFormatManager& formatManagerToUse,
                              juce::AudioThumbnailCache& cacheToUse)
    : id (deckId),
      mirrored (mirroredToUse),
      useHotCuePads (useHotCuePadsToUse),
      player (playerToUse),
      header (deckId),
      waveformDisplay (formatManagerToUse, cacheToUse),
      jogWheel (mirroredToUse)
{
    addAndMakeVisible (header);
    addAndMakeVisible (waveformDisplay);
    addAndMakeVisible (jogWheel);

    // Só a grade correspondente a este deck fica visível.
    addAndMakeVisible (beatJump);
    addAndMakeVisible (hotCuePads);
    beatJump.setVisible (! useHotCuePads);
    hotCuePads.setVisible (useHotCuePads);

    // ---- Transport ----
    playButton.addListener (this);
    playButton.setTooltip ("Iniciar / pausar reproducao");
    addAndMakeVisible (playButton);

    stopButton.addListener (this);
    stopButton.setTooltip ("Parar reproducao");
    addAndMakeVisible (stopButton);

    forwardButton.addListener (this);
    forwardButton.setTooltip ("Avancar 5%");
    addAndMakeVisible (forwardButton);

    reverseButton.addListener (this);
    reverseButton.setTooltip ("Retroceder 5%");
    addAndMakeVisible (reverseButton);

    cueButton.setClickingTogglesState (true);
    cueButton.setTooltip ("Monitoramento de fone (PFL)");
    addAndMakeVisible (cueButton);

    // ---- Velocidade ----
    speedSlider.setRange (0.5, 3.0);
    speedSlider.setValue (1.0);
    speedSlider.addListener (this);
    addAndMakeVisible (speedSlider);

    // ---- Jog wheel: scratch desloca a posicao ----
    jogWheel.onScratch = [this] (float deltaRadians)
    {
        if (player == nullptr)
            return;

        // ~0.15 s por radiano.
        player->setPosition (player->getPositionInSeconds() + deltaRadians * 0.15);
    };

    // ---- Beat jump: move em fracoes de batida ----
    beatJump.onBeatJump = [this] (double fraction)
    {
        if (player == nullptr)
            return;

        // Sem BPM conhecido, assume 120 BPM: uma batida = 0.5 s.
        // Quando a deteccao de BPM entrar, este valor passa a usar o BPM real.
        const auto bpm = player->getBpm() > 0.0 ? player->getBpm() : 120.0;
        const auto secondsPerBeat = 60.0 / bpm;

        player->setPosition (player->getPositionInSeconds() + fraction * secondsPerBeat);
    };

    // ---- Hot cue: arma um cue ou salta para ele ----
    hotCuePads.onCueClicked = [this] (int padIndex, double seconds, bool isSetting)
    {
        if (player == nullptr)
            return;

        if (isSetting)
        {
            // Arma um cue novo na posicao atual.
            const auto position = player->getPositionInSeconds();
            hotCuePads.setCueTime (padIndex, position);
        }
        else if (seconds >= 0.0)
        {
            // Salta para o cue existente.
            player->setPosition (seconds);
        }
    };

    // Arrastar-soltar de arquivos.
    //
    // IMPORTANTE para o drag-and-drop: o JUCE localiza o alvo do drop via
    // Component::getComponentAt(), que consulta Component::hitTest(). Com
    // setInterceptsMouseClicks(false, ...), o hitTest() do deck so retorna
    // true se algum FILHO estiver sob o cursor — e getComponentAt() devolvia
    // nullptr nas areas do deck sem nenhum filho, o que fazia o drop falhar
    // silenciosamente.
    //
    // Com (true, true) o deck inteiro e um alvo de hit-test valido, e os
    // botoes/sliders filhos continuam recebendo os cliques normalmente.
    setInterceptsMouseClicks (true, true);
    setWantsKeyboardFocus (false);

    // Timer para atualizar playhead (~30 fps)
    startTimer (33);
}

DeckComponent::~DeckComponent()
{
    stopTimer();
}

//==============================================================================
void DeckComponent::paint (juce::Graphics& g)
{
    g.fillAll (RekordboxColours::panel);

    // Feedback visual quando uma faixa esta sendo arrastada sobre este deck.
    if (dragOverActive)
    {
        g.setColour (RekordboxColours::accent.withAlpha (0.25f));
        g.fillRect (getLocalBounds());

        g.setColour (RekordboxColours::accent);
        g.drawRect (getLocalBounds(), 2);

        g.setFont (juce::Font (juce::FontOptions (18.0f)));
        g.setColour (RekordboxColours::textPrimary);
        g.drawText ("Solte para carregar", getLocalBounds(), juce::Justification::centred, true);
    }
}

//==============================================================================
void DeckComponent::resized()
{
    auto area = getLocalBounds().reduced (2);

    // ---- Cabecalho: ~64 px ----
    header.setBounds (area.removeFromTop (64));

    // ---- Faixa de transporte no rodape: ~44 px ----
    auto transportArea = area.removeFromBottom (44);

    // Os botoes de transporte ficam na lateral do jog wheel.
    auto playArea = transportArea.removeFromLeft (transportArea.getWidth() / 3);
    auto buttonRow = playArea.removeFromTop (24);

    const auto buttonWidth = buttonRow.getWidth() / 2;

    playButton.setBounds (buttonRow.removeFromLeft (buttonWidth));
    stopButton.setBounds (buttonRow.removeFromLeft (buttonWidth));

    auto navRow = playArea.removeFromTop (24);
    const auto navWidth = navRow.getWidth() / 2;

    reverseButton.setBounds (navRow.removeFromLeft (navWidth));
    forwardButton.setBounds (navRow.removeFromLeft (navWidth));

    // Velocidade na faixa restante.
    speedSlider.setBounds (playArea.reduced (2));

    // ---- Area central: pads + jog wheel ----
    auto middleArea = area;

    // O jog wheel e quadrado, com ~140 px de diametro.
    const auto jogSize = juce::jmin (140, juce::jmin (middleArea.getWidth(), middleArea.getHeight()));
    auto jogArea = middleArea.withSizeKeepingCentre (jogSize, jogSize);

    // No deck 1 o jog fica a DIREITA e os pads a ESQUERDA; no deck 2 e o
    // inverso. O `mirrored` decide — sem `if` espalhado no desenho.
    if (! mirrored)
        jogArea = middleArea.removeFromRight (jogSize);
    else
        jogArea = middleArea.removeFromLeft (jogSize);

    jogWheel.setBounds (jogArea);

    // Os pads ocupam o resto da area central.
    if (! useHotCuePads)
        beatJump.setBounds (middleArea);
    else
        hotCuePads.setBounds (middleArea);

    // ---- CUE: ao lado do jog wheel ----
    auto cueArea = juce::Rectangle<int> (jogArea.getX() - 40, jogArea.getY(), 38, 28);

    if (mirrored)
        cueArea = juce::Rectangle<int> (jogArea.getRight() + 2, jogArea.getY(), 38, 28);

    cueArea = cueArea.getIntersection (area);

    if (cueArea.getWidth() > 8 && cueArea.getHeight() > 8)
        cueButton.setBounds (cueArea);
    else
        cueButton.setBounds (0, 0, 0, 0);

    // ---- Waveform: ocupa o espaco entre o cabecalho e a area central ----
    auto waveformArea = area;
    waveformArea.removeFromBottom (juce::jmin (120, waveformArea.getHeight() / 2));
    waveformDisplay.setBounds (waveformArea.reduced (2));
}

//==============================================================================
void DeckComponent::buttonClicked (juce::Button* button)
{
    if (player == nullptr)
        return;

    if (button == &playButton)
    {
        if (player->isPlaying())
            player->stop();
        else
            player->start();
    }
    else if (button == &stopButton)
    {
        player->stop();
        player->setPositionRelative (0.0);
    }
    else if (button == &forwardButton)
    {
        const auto pos = player->getPositionRelative();
        if (pos < 0.95)
            player->setPositionRelative (pos + 0.05);
    }
    else if (button == &reverseButton)
    {
        const auto pos = player->getPositionRelative();
        if (pos > 0.05)
            player->setPositionRelative(pos - 0.05);
    }
    // --- NOVO: Handler para o botao CUE (PFL / monitoramento de fone) ---
    else if (button == &cueButton)
    {
        if (player != nullptr)
            player->setCue(cueButton.getToggleState());
    }
}

void DeckComponent::sliderValueChanged(juce::Slider* slider)
{
    if (player == nullptr)
        return;

    if (slider == &speedSlider)
        player->setSpeed (speedSlider.getValue());
}

//==============================================================================
bool DeckComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    return files.size() == 1;
}

void DeckComponent::filesDropped (const juce::StringArray& files, int, int)
{
    if (files.size() == 1)
        loadFileURL (juce::URL { juce::File { files[0] } }, juce::String(), juce::String());
}

//==============================================================================
// Drag-and-drop interno: arrastar uma faixa da playlist para este deck.
//
// O payload e um PlaylistDragPayload (ReferenceCountedObject) e nao uma String
// crua. Isso e essencial: uma String seria ambigua com o drop de texto/arquivo
// vindo de fora do app. O drop de arquivos do sistema operacional continua
// passando por filesDropped(), num caminho separado.

bool DeckComponent::isInterestedInDragSource (const juce::DragAndDropTarget::SourceDetails& details)
{
    return PlaylistDragPayload::isPayload (details.description);
}

void DeckComponent::itemDragEnter (const juce::DragAndDropTarget::SourceDetails& details)
{
    if (isInterestedInDragSource (details))
    {
        dragOverActive = true;
        repaint();
    }
}

void DeckComponent::itemDragExit (const juce::DragAndDropTarget::SourceDetails&)
{
    dragOverActive = false;
    repaint();
}

void DeckComponent::itemDropped (const juce::DragAndDropTarget::SourceDetails& details)
{
    dragOverActive = false;
    repaint();

    if (auto* payload = PlaylistDragPayload::getFrom (details.description))
    {
        if (payload->file.existsAsFile())
            loadFileURL(juce::URL { payload->file }, juce::String(), juce::String());
    }
}

//==============================================================================
void DeckComponent::timerCallback()
{
    if (player == nullptr)
        return;

    waveformDisplay.setPositionRelative (player->getPositionRelative());
    jogWheel.setPositionRelative (player->getPositionRelative());

    updateHeaderTimes();

    // O BPM ainda nao e detectado — o placeholder fica "--.--" ate o
    // detector de BPM entrar (ver docs/ROADMAP.md).
    header.setBpm (player->getBpm());
    jogWheel.setBpm (player->getBpm());
}

//==============================================================================
void DeckComponent::updateHeaderTimes()
{
    if (player == nullptr)
        return;

    const auto elapsed = player->getPositionInSeconds();
    const auto length = player->getLengthInSeconds();

    // Decorrido, contagem regressiva e duracao total.
    header.setTimes (elapsed, length > 0.0 ? length - elapsed : 0.0, length);
}

//==============================================================================
void DeckComponent::loadFile (juce::URL audioURL, double bpm)
{
    loadFileURL (audioURL, juce::String(), juce::String());

    // BPM lido da faixa: publica no player (para o jog wheel calcular o pitch
    // efetivo) e no proprio jog wheel, que exibe o valor no centro do disco.
    if (player != nullptr)
        player->setBpm (bpm);
    jogWheel.setBpm (bpm);
}

void DeckComponent::loadFile (juce::URL audioURL, double bpm,
                              const juce::String& title, const juce::String& artist)
{
    loadFileURL (audioURL, title, artist);

    if (player != nullptr)
        player->setBpm (bpm);
    jogWheel.setBpm (bpm);
}

void DeckComponent::loadFileURL (juce::URL audioURL, const juce::String& title, const juce::String& artist)
{
    if (player == nullptr)
        return;

    player->loadURL (audioURL);
    waveformDisplay.loadURL (audioURL);

    // Usa os metadados reais do TagLib quando disponiveis; caso contrario, o nome do arquivo.
    header.setTrackTitle (title.isNotEmpty() ? title : juce::File (audioURL.getLocalFile()).getFileNameWithoutExtension());
    header.setTrackArtist (artist);
    header.setBpm (player->getBpm());

    updateHeaderTimes();
}
