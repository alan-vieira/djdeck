#include "DeckComponent.h"
#include "LayoutConfig.hpp"
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
      header (deckId, mirrored),
      waveformDisplay (formatManagerToUse, cacheToUse),
      jogWheel (mirroredToUse)
{
    addAndMakeVisible (header);
    addAndMakeVisible (waveformDisplay);
    addAndMakeVisible (jogWheel);

    // ---- Transport ----
    playButton.addListener (this);
    playButton.setTooltip ("Iniciar / pausar reproducao");
    playButton.setName ("deckPlay");
    addAndMakeVisible (playButton);

    cueButton.setClickingTogglesState (true);
    cueButton.setTooltip ("Monitoramento de fone (PFL)");
    cueButton.setName ("deckCue");
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
    g.fillAll (djdeck::layout::colours::Panel);

    // Feedback visual quando uma faixa esta sendo arrastada sobre este deck.
    if (dragOverActive)
    {
        g.setColour (djdeck::layout::colours::Accent.withAlpha (0.25f));
        g.fillRect (getLocalBounds());

        g.setColour (djdeck::layout::colours::Accent);
        g.drawRect (getLocalBounds(), 2);

        g.setFont (juce::Font (juce::FontOptions (18.0f)));
        g.setColour (djdeck::layout::colours::Primary);
        g.drawText ("Solte para carregar", getLocalBounds(), juce::Justification::centred, true);
    }
}

//==============================================================================
void DeckComponent::resized()
{
    auto area = getLocalBounds().reduced (2);

    // Posicao absoluta do deck no canvas do Figma (para calcular posicoes relativas dos filhos)
    auto deckAbsX = ! mirrored ? djdeck::layout::figmaNodedeck1::xPx
                                     : djdeck::layout::figmaNodedeck2::xPx;
    auto deckAbsY = ! mirrored ? djdeck::layout::figmaNodedeck1::yPx
                                     : djdeck::layout::figmaNodedeck2::yPx;

    // Passa a posicao absoluta do deck para o cabecalho (posicionamento dos textos)
    header.setDeckPosition (static_cast<int> (deckAbsX), static_cast<int> (deckAbsY));

    // ---- Cabecalho: altura do nó do Figma (deckHeader) ----
    // Posicionado em (0, 0) relativo ao deck, com altura do nó do Figma.
    header.setBounds (0, 0, area.getWidth(), djdeck::layout::deckHeader::heightPx);

    // ---- Miniwaveform: abaixo do cabecalho (miniwaveform1 ou miniwaveform2 relativo ao deck) ----
    auto mwNodeX = ! mirrored ? djdeck::layout::figmaNodeminiwaveform1::xPx
                                    : djdeck::layout::figmaNodeminiwaveform2::xPx;
    auto mwNodeY = ! mirrored ? djdeck::layout::figmaNodeminiwaveform1::yPx
                                    : djdeck::layout::figmaNodeminiwaveform2::yPx;
    auto mwNodeW = ! mirrored ? djdeck::layout::figmaNodeminiwaveform1::widthPx
                                    : djdeck::layout::figmaNodeminiwaveform2::widthPx;
    auto mwNodeH = ! mirrored ? djdeck::layout::figmaNodeminiwaveform1::heightPx
                                    : djdeck::layout::figmaNodeminiwaveform2::heightPx;

    const auto mwRelX = static_cast<int> (mwNodeX - deckAbsX);
    const auto mwRelY = static_cast<int> (mwNodeY - deckAbsY);
    const auto mwW    = static_cast<int> (mwNodeW);
    const auto mwH    = static_cast<int> (mwNodeH);

    // Sem `reduced (2)`: o no do Figma ja mede 670 x 34 exatamente na
    // posicao 0,181 (relativo ao deck: 0,44). A reducao de 2 px encolhia e
    // deslocava a onda em relacao ao cabecalho e ao deck.
    waveformDisplay.setBounds (juce::Rectangle<int> (mwRelX, mwRelY, mwW, mwH));

    // ---- Transport no rodape: altura do nó do Figma ----
    auto transportArea = juce::Rectangle<int> (0, area.getHeight() - djdeck::layout::deckTransport::heightPx,
                                               area.getWidth(), djdeck::layout::deckTransport::heightPx);

    // ---- Jog wheel, CUE, Play e pitch (todos a partir dos nos do Figma, relativos ao deck) ----

        // Jog wheel: posicao relativa ao deck (jog1 para deck 1, jog2 para deck 2)
    auto jogNodeX = ! mirrored ? djdeck::layout::figmaNodejog1::xPx
                                     : djdeck::layout::figmaNodejog2::xPx;
    auto jogNodeY = ! mirrored ? djdeck::layout::figmaNodejog1::yPx
                                     : djdeck::layout::figmaNodejog2::yPx;
    auto jogNodeW = ! mirrored ? djdeck::layout::figmaNodejog1::widthPx
                                     : djdeck::layout::figmaNodejog2::widthPx;
    auto jogNodeH = ! mirrored ? djdeck::layout::figmaNodejog1::heightPx
                                     : djdeck::layout::figmaNodejog2::heightPx;

    const auto jogRelX = static_cast<int> (jogNodeX - deckAbsX);
    const auto jogRelY = static_cast<int> (jogNodeY - deckAbsY);
    const auto jogSize = static_cast<int> (jogNodeW);

    auto jogArea = juce::Rectangle<int> (jogRelX, jogRelY, jogSize, jogSize);
    jogWheel.setBounds (jogArea);

    // Pitch slider: posicao absoluta do Figma convertida para relativa ao deck.
    // Fica SEMPRE à direita do jog wheel nos dois decks (o espelhamento inverte o
    // lado do jog, o pitch acompanha o jog).
        auto pitchNodeX = ! mirrored ? djdeck::layout::figmaNodepitch_slider_1::xPx
                                           : djdeck::layout::figmaNodepitch_slider_2::xPx;
        auto pitchNodeY = ! mirrored ? djdeck::layout::figmaNodepitch_slider_1::yPx
                                           : djdeck::layout::figmaNodepitch_slider_2::yPx;
        auto pitchNodeW = ! mirrored ? djdeck::layout::figmaNodepitch_slider_1::widthPx
                                           : djdeck::layout::figmaNodepitch_slider_2::widthPx;
        auto pitchNodeH = ! mirrored ? djdeck::layout::figmaNodepitch_slider_1::heightPx
                                           : djdeck::layout::figmaNodepitch_slider_2::heightPx;

        const auto pitchRelX = static_cast<int> (pitchNodeX - deckAbsX);
        const auto pitchRelY = static_cast<int> (pitchNodeY - deckAbsY);
        const auto pitchW    = static_cast<int> (juce::roundToInt (pitchNodeW));
        const auto pitchH    = static_cast<int> (juce::roundToInt (pitchNodeH));

        pitchSlider.setBounds (pitchRelX, pitchRelY, pitchW, pitchH);

    // CUE e Play: posicoes relativas ao deck (CUE1/play1 para deck 1, cue2/play2 para deck 2)
    auto cueNodeX = ! mirrored ? djdeck::layout::figmaNodecue1::xPx
                                     : djdeck::layout::figmaNodecue2::xPx;
    auto cueNodeY = ! mirrored ? djdeck::layout::figmaNodecue1::yPx
                                     : djdeck::layout::figmaNodecue2::yPx;
    auto playNodeX = ! mirrored ? djdeck::layout::figmaNodeplay1::xPx
                                      : djdeck::layout::figmaNodeplay2::xPx;
    auto playNodeY = ! mirrored ? djdeck::layout::figmaNodeplay1::yPx
                                      : djdeck::layout::figmaNodeplay2::yPx;
    const auto buttonSize = djdeck::layout::deckCue::widthPx;

    const auto cueRelX  = static_cast<int> (cueNodeX  - deckAbsX);
    const auto cueRelY  = static_cast<int> (cueNodeY  - deckAbsY);
    const auto playRelX = static_cast<int> (playNodeX - deckAbsX);
    const auto playRelY = static_cast<int> (playNodeY - deckAbsY);

    playButton.setBounds (playRelX, playRelY, buttonSize, buttonSize);
    cueButton.setBounds  (cueRelX,  cueRelY,  buttonSize, buttonSize);
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
    // --- Handler para o botao CUE (PFL / monitoramento de fone) ---
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
