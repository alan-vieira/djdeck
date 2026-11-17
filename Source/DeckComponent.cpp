#include "DeckComponent.h"
#include "RekordboxColours.h"
#include "TagLibReader.h"
#include "TrackMetadata.h"

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

    // ---- Velocidade (pitch) ----
    pitchSlider.setRange (-16.0, 16.0, 0.1);
    pitchSlider.setValue (0.0);
    // Nomeia o slider: o LookAndFeel usa este nome para desenhar os pinos de
    // referencia apenas no pitch (e nao nos faders de volume/EQ do mixer).
    pitchSlider.setName ("pitch");
    pitchSlider.setTooltip ("Pitch / velocidade (+-16%)");
    pitchSlider.setDoubleClickReturnValue (true, 0.0);
    pitchSlider.addListener (this);
    addAndMakeVisible (pitchSlider);

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

    // Quando o BPM for detectado (por tag ou por analise de audio), publica no
    // cabecalho e no jog wheel.
    // SafePointer: o handler roda na message thread e pode chegar DEPOIS do
    // DeckComponent ter sido destruido (fechamento do app). O SafePointer vira
    // null nesse caso, evitando o use-after-free.
    juce::Component::SafePointer<DeckComponent> safeThis (this);

    player->setBpmDetectedHandler ([safeThis] (double bpm)
    {
        juce::Logger::writeToLog ("[DIAG] DeckComponent bpmDetectedHandler called with " + juce::String (bpm, 2));
        if (safeThis == nullptr)
            return;

        safeThis->player->setBpm (bpm);
        safeThis->header.setBpm (bpm);
        safeThis->jogWheel.setBpm (bpm);
        // Propaga o BPM detectado para a biblioteca, se houver um listener registrado
        if (safeThis->onBpmDetected != nullptr)
            safeThis->onBpmDetected (safeThis->id, bpm);
        safeThis->jogWheel.setBpm (bpm);
    });
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

    // ---- Area central: pads + jog wheel + pitch slider ----
    auto middleArea = area;

    // O jog wheel e quadrado, com ~140 px de diametro.
    const auto jogSize = juce::jmin (140, juce::jmin (middleArea.getWidth(), middleArea.getHeight()));
    const auto pitchWidth = 48;
    const auto cueWidth = 40;

    if (! mirrored)
    {
        // Deck 1 (nao espelhado): [pads] [CUE] [jog] [pitch]
        // Pitch fica à direita do jog wheel.
        auto pitchArea = middleArea.removeFromRight (pitchWidth);
        pitchArea = pitchArea.withSizeKeepingCentre (pitchWidth, jogSize);
        auto jogArea = middleArea.removeFromRight (jogSize);
        jogArea = jogArea.withSizeKeepingCentre (jogSize, jogSize);
        auto cueStrip = middleArea.removeFromRight (cueWidth);
        auto cueArea = juce::Rectangle<int> (cueStrip.getX() + (cueStrip.getWidth() - 38) / 2,
                                             jogArea.getY(), 38, 28);

        jogWheel.setBounds (jogArea);
        pitchSlider.setBounds (pitchArea.reduced (2));
        cueButton.setBounds (cueArea);
    }
    else
    {
        // Deck 2 (espelhado): [jog] [pitch] [CUE] [pads]
        // Pitch fica à direita do jog wheel.
        auto jogArea = middleArea.removeFromLeft (jogSize);
        jogArea = jogArea.withSizeKeepingCentre (jogSize, jogSize);

        auto pitchArea = middleArea.removeFromLeft (pitchWidth);
        pitchArea = pitchArea.withSizeKeepingCentre (pitchWidth, jogSize);

        auto cueStrip = middleArea.removeFromLeft (cueWidth);
        auto cueArea = juce::Rectangle<int> (cueStrip.getX() + (cueStrip.getWidth() - 38) / 2,
                                             jogArea.getY(), 38, 28);

        jogWheel.setBounds (jogArea);
        pitchSlider.setBounds (pitchArea.reduced (2));
        cueButton.setBounds (cueArea);
    }

    // Os pads ocupam o resto da area central.
    if (! useHotCuePads)
        beatJump.setBounds (middleArea);
    else
        hotCuePads.setBounds (middleArea);

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

    if (slider == &pitchSlider)
    {
        // Converte variacao de pitch (%) em taxa de reproducao:
        //   0%   -> 1.0x  (neutro)
        //  +16%  -> 1.16x  (padrao industria: faixa +-16%)
        //  -16%  -> 0.84x
        // DeckPlayer::setSpeed limita a 0.01x .. 2.0x.
        player->setSpeed (1.0 + slider->getValue() / 100.0);
    }
}

//==============================================================================
bool DeckComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    return files.size() == 1;
}

void DeckComponent::filesDropped (const juce::StringArray& files, int, int)
{
    if (files.size() == 1)
        loadFileURL (juce::URL { juce::File { files[0] } }, 0.0, juce::String(), juce::String());
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
            loadFileURL (juce::URL { payload->file }, payload->bpm,
                         payload->title, payload->artist);
        // Notificar a big waveform para que ela exiba o track.
        // O drag and drop ignora o caminho do double-click, mas a big
        // waveform e notificada via onTrackLoaded definido pelo MainComponent.
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
    juce::Logger::writeToLog ("[DIAG] DeckComponent::loadFile called with bpm=" + juce::String (bpm, 2));

    // Define o BPM no player ANTES de carregar, para que loadFileURL veja
    // getBpm() > 0 e nao dispare detectBpm desnecessariamente, e para que o
    // header ja receba o valor correto na primeira pintura.
    if (player != nullptr)
        player->setBpm (bpm);
    jogWheel.setBpm (bpm);
    header.setBpm (bpm);

    loadFileURL (audioURL, bpm, juce::String(), juce::String());
}

void DeckComponent::loadFile (juce::URL audioURL, double bpm,
                              const juce::String& title, const juce::String& artist)
{
    juce::Logger::writeToLog ("[DIAG] DeckComponent::loadFile(url,bpm,title,artist) called with bpm=" + juce::String (bpm, 2) + " title=" + title);

    if (player != nullptr)
        player->setBpm (bpm);
    jogWheel.setBpm (bpm);
    header.setBpm (bpm);

    loadFileURL (audioURL, bpm, title, artist);
}

void DeckComponent::loadFileURL (juce::URL audioURL, double bpm,
                              const juce::String& title, const juce::String& artist)
{
    if (player == nullptr)
        return;

    // Sempre reseta o pitch ao carregar uma faixa nova: o usuario espera
    // tocar a proxima faixa na velocidade original (1.0x / 0%).
    pitchSlider.setValue (0.0);
    player->setSpeed (1.0);

    juce::Logger::writeToLog ("[DIAG] DeckComponent::loadFileURL called, player->getBpm()=" + juce::String (player->getBpm(), 2));

    // Se o caller forneceu um BPM (duplo-clique na biblioteca, ou drag-and-drop
    // com payload preenchido), garante que o player ja saiba o valor antes
    // de carregar o audio.
    if (bpm > 0.0)
        player->setBpm (bpm);

    player->loadURL (audioURL);
    waveformDisplay.loadURL (audioURL);
    // Notifica quem coordena o layout (o MainComponent) para que a big
    // waveform do topo carregue a mesma faixa. Dispara em TODOS os caminhos
    // de carga, porque loadFileURL e o ponto de convergencia.
    if (onTrackLoaded != nullptr)
        onTrackLoaded (id, audioURL);

    // Se nenhum BPM foi fornecido (drag-and-drop de arquivo do sistema,
    // ou payload sem metadados), tenta ler do TagLib antes de recorrer
    // à detecção por análise de áudio.
    auto localTitle = title;
    auto localArtist = artist;
    if (player->getBpm() <= 0.0)
    {
        TagLibReader tagReader;
        auto metadata = tagReader.readMetadata (audioURL.getLocalFile());
        if (metadata.isValid && metadata.bpm > 0.0)
        {
            player->setBpm (metadata.bpm);
            if (localTitle.isEmpty())
                localTitle = metadata.title;
            if (localArtist.isEmpty())
                localArtist = metadata.artist;
        }
    }

    // Se ainda nao ha BPM conhecido (sem tag na faixa nem valor da biblioteca),
    // dispara a deteccao por analise de audio. Quando termina, o handler
    // definido no construtor atualiza o cabecalho e o jog wheel.
    if (player->getBpm() <= 0.0)
        player->detectBpm (audioURL.getLocalFile());

    // Usa os metadados reais do TagLib quando disponiveis; caso contrario, o nome do arquivo.
    header.setTrackTitle (localTitle.isNotEmpty() ? localTitle : juce::File (audioURL.getLocalFile()).getFileNameWithoutExtension());
    header.setTrackArtist (localArtist);
    header.setBpm (player->getBpm());

    juce::Logger::writeToLog ("[DIAG] DeckComponent::loadFileURL finished, player->getBpm()=" + juce::String (player->getBpm(), 2));

    updateHeaderTimes();
}
