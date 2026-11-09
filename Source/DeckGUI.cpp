#include "DeckGUI.h"

//==============================================================================
DeckGUI::DeckGUI(int deckId,
                 DeckPlayer* playerToUse,
                 juce::AudioFormatManager& formatManagerToUse,
                 juce::AudioThumbnailCache& cacheToUse)
    : id(deckId),
      player(playerToUse),
      waveformDisplay(formatManagerToUse, cacheToUse)
{
    // Botões de transporte
    addAndMakeVisible(playButton);
    playButton.addListener(this);
    playButton.setTooltip("Iniciar / pausar reprodução");

    addAndMakeVisible(stopButton);
    stopButton.addListener(this);
    stopButton.setTooltip("Parar reprodução");

    addAndMakeVisible(forwardButton);
    forwardButton.addListener(this);
    forwardButton.setTooltip("Avançar 5%");

    addAndMakeVisible(reverseButton);
    reverseButton.addListener(this);
    reverseButton.setTooltip("Retroceder 5%");

    // Sliders
    addAndMakeVisible(volLabel);
    addAndMakeVisible(volSlider);
    volSlider.addListener(this);
    volSlider.setRange(0.0, 1.0);
    volSlider.setValue(0.5);
    volSlider.setNumDecimalPlacesToDisplay(2);
    volLabel.attachToComponent(&volSlider, true);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);

    addAndMakeVisible(speedLabel);
    addAndMakeVisible(speedSlider);
    speedSlider.addListener(this);
    speedSlider.setRange(0.5, 3.0);
    speedSlider.setValue(1.0);
    speedSlider.setNumDecimalPlacesToDisplay(2);
    speedLabel.attachToComponent(&speedSlider, true);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);

    addAndMakeVisible(posLabel);
    addAndMakeVisible(posSlider);
    posSlider.addListener(this);
    posSlider.setRange(0.0, 1.0);
    posSlider.setValue(0.0);
    posSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    posLabel.attachToComponent(&posSlider, true);

    // Waveform
    addAndMakeVisible(waveformDisplay);

    // Arrastar-soltar de arquivos.
    //
    // IMPORTANTE para o drag-and-drop: o JUCE localiza o alvo do drop via
    // Component::getComponentAt(), que consulta Component::hitTest(). Com
    // setInterceptsMouseClicks(false, ...), o hitTest() do DeckGUI só retorna
    // true se algum FILHO estiver sob o cursor — e getComponentAt() devolvia
    // nullptr nas áreas do deck sem nenhum filho (vão entre os sliders e a
    // waveform), o que fazia o drop falhar silenciosamente.
    //
    // Com (true, true) o DeckGUI inteiro é um alvo de hit-test válido, e os
    // botões/sliders filhos continuam recebendo os cliques normalmente.
    setInterceptsMouseClicks(true, true);
    setWantsKeyboardFocus(false);

    // Timer para atualizar playhead (~30 fps)
    startTimer(33);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

//==============================================================================
void DeckGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Deck " + juce::String(id),
               getLocalBounds().withHeight(30),
               juce::Justification::centredTop, true);

    // Feedback visual quando uma faixa está sendo arrastada sobre este deck.
    if (dragOverActive)
    {
        g.setColour(juce::Colours::lightblue.withAlpha(0.25f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::lightblue);
        g.drawRect(getLocalBounds(), 2);
        g.setFont(18.0f);
        g.drawText("Solte para carregar",
                   getLocalBounds(),
                   juce::Justification::centred, true);
    }
}

void DeckGUI::resized()
{
    auto area = getLocalBounds();
    int h = area.getHeight();
    int row = h / 14;

    // Botões de transporte lado a lado (4 colunas na mesma faixa).
    // Cada removeFromLeft() devolve um NOVO retângulo, então é preciso
    // encadear sobre o mesmo retângulo para dividir a faixa em 4 partes.
    auto buttonRow = area.removeFromTop(row);
    const int buttonWidth = buttonRow.getWidth() / 4;

    playButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    stopButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    reverseButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    forwardButton.setBounds(buttonRow.removeFromLeft(buttonWidth));

    // Sliders linha superior
    auto sliderRow = area.removeFromTop(row);
    const int sliderWidth = sliderRow.getWidth() / 3;

    volSlider.setBounds(sliderRow.removeFromLeft(sliderWidth));
    speedSlider.setBounds(sliderRow.removeFromLeft(sliderWidth));

    // Posição
    posSlider.setBounds(area.removeFromTop(row));

    // Waveform ocupa o resto
    waveformDisplay.setBounds(area);
}

//==============================================================================
void DeckGUI::buttonClicked(juce::Button* button)
{
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
        player->setPositionRelative(0.0);
    }
    else if (button == &forwardButton)
    {
        auto pos = player->getPositionRelative();
        if (pos < 0.95)
            player->setPositionRelative(pos + 0.05);
    }
    else if (button == &reverseButton)
    {
        auto pos = player->getPositionRelative();
        if (pos > 0.05)
            player->setPositionRelative(pos - 0.05);
    }
}

void DeckGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volSlider)
        player->setGain(volSlider.getValue());
    else if (slider == &speedSlider)
        player->setSpeed(speedSlider.getValue());
    else if (slider == &posSlider)
        player->setPositionRelative(posSlider.getValue());
}

//==============================================================================
bool DeckGUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    return files.size() == 1;
}

void DeckGUI::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    if (files.size() == 1)
        loadFileURL(juce::URL{ juce::File{ files[0] } });
}

//==============================================================================
// Drag-and-drop interno: arrastar uma faixa da playlist para este deck.
//
// O payload é um PlaylistDragPayload (ReferenceCountedObject) e não uma String
// crua. Isso é essencial: uma String seria ambígua com o drop de texto/arquivo
// vindo de fora do app. O drop de arquivos do sistema operacional continua
// passando por filesDropped(), num caminho separado.

bool DeckGUI::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    return PlaylistDragPayload::isPayload (details.description);
}

void DeckGUI::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& details)
{
    if (isInterestedInDragSource (details))
    {
        dragOverActive = true;
        repaint();
    }
}

void DeckGUI::itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    dragOverActive = false;
    repaint();
}

void DeckGUI::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    dragOverActive = false;
    repaint();

    if (auto* payload = PlaylistDragPayload::getFrom (details.description))
    {
        if (payload->file.existsAsFile())
            loadFileURL (juce::URL { payload->file });
    }
}

//==============================================================================
void DeckGUI::timerCallback()
{
    if (player != nullptr)
    {
        waveformDisplay.setPositionRelative(player->getPositionRelative());
        posSlider.setValue(player->getPositionRelative(), juce::dontSendNotification);
    }
}

void DeckGUI::loadFile(juce::URL audioURL)
{
    loadFileURL(audioURL);
}

void DeckGUI::loadFileURL(juce::URL audioURL)
{
    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);
}
