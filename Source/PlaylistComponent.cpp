#include "PlaylistComponent.h"

// IDs das colunas da tabela
enum PlaylistColumns
{
    titleColumn = 1,
    artistColumn,
    lengthColumn
};

//==============================================================================
PlaylistComponent::PlaylistComponent()
{
    addAndMakeVisible(playlistTable);
    playlistTable.setModel(this);
    playlistTable.getHeader().addColumn("Título", titleColumn, 200);
    playlistTable.getHeader().addColumn("Artista", artistColumn, 150);
    playlistTable.getHeader().addColumn("Duração", lengthColumn, 100);

    addAndMakeVisible(addButton);
    addButton.addListener(this);
    addButton.setTooltip("Abrir arquivo(s) de áudio");

    // Detecta o duplo-clique nas linhas da tabela. A TableListBox não expõe
    // um callback para isso, então usamos um MouseListener registrado nela.
    // wantsEventsForAllNestedChildComponents = true porque as linhas são
    // componentes filhos internos da ListBox.
    playlistTable.addMouseListener (this, true);
}

PlaylistComponent::~PlaylistComponent()
{
    playlistTable.setModel(nullptr);
}

//==============================================================================
void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds();

    addButton.setBounds(area.removeFromTop(40).removeFromLeft(200));
    playlistTable.setBounds(area);
}

//==============================================================================
int PlaylistComponent::getNumRows()
{
    return static_cast<int>(songs.size());
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/,
                                           int /*height*/, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::darkblue);
    else if (rowNumber % 2 == 1)
        g.fillAll(juce::Colours::darkgrey);
    else
        g.fillAll(juce::Colours::black);
}

void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId,
                                  int /*width*/, int /*height*/, bool /*rowIsSelected*/)
{
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);

    const auto& song = songs.getReference(rowNumber);

    switch (columnId)
    {
    case titleColumn:   g.drawText(song.title, 2, 0, 190, 20, juce::Justification::centredLeft, true); break;
    case artistColumn:  g.drawText(song.artist, 2, 0, 140, 20, juce::Justification::centredLeft, true); break;
    case lengthColumn:
    {
        auto mins = static_cast<int>(song.lengthInSeconds) / 60;
        auto secs = static_cast<int>(song.lengthInSeconds) % 60;
        g.drawText(juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2),
                   2, 0, 90, 20, juce::Justification::centredLeft, true);
        break;
    }
    default: break;
    }
}

juce::Component* PlaylistComponent::refreshComponentForCell(int /*rowNumber*/, int /*columnId*/,
                                                            bool /*rowIsSelected*/,
                                                            juce::Component* existingComponentToUpdate)
{
    return existingComponentToUpdate;
}

void PlaylistComponent::selectedRowsChanged(int lastRowSelected)
{
    juce::ignoreUnused(lastRowSelected);

    // Apenas GUARDA a seleção. NÃO carrega a faixa aqui de propósito:
    // se carregássemos nesta callback, o simples mouse-down do ARRASTE já
    // dispararia o carregamento no Deck 1, e o drop no Deck 2 carregaria de
    // novo — terminando com a mesma faixa nos dois decks.
    //
    // Carregamento acontece apenas em:
    //   - duplo-clique na linha  -> Deck 1
    //   - arrastar para um deck  -> o deck de destino
    selectedRow = playlistTable.getSelectedRow();
}

void PlaylistComponent::sortOrderChanged(int /*newSortColumnId*/, bool /*isForwards*/)
{
    playlistTable.updateContent();
}

//==============================================================================
void PlaylistComponent::buttonClicked(juce::Button* button)
{
    if (button == &addButton)
    {
        fileChooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems,
                                [this](const juce::FileChooser& chooser)
                                {
                                    auto files = chooser.getResults();
                                    for (auto& f : files)
                                    {
                                        Song song{ f };
                                        // Tenta obter duração via format manager seria necessário acesso ao formatManager
                                        // Por enquanto, duração fica 0
                                        songs.add(song);
                                    }
                                    playlistTable.updateContent();
                                });
    }
}

//==============================================================================
bool PlaylistComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Aceita apenas arquivos de áudio conhecidos pelo seletor.
    juce::ignoreUnused (files);
    return true;
}

void PlaylistComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (auto& f : files)
    {
        Song song{ juce::File(f) };
        songs.add(song);
    }
    playlistTable.updateContent();
}

//==============================================================================
// Drag-and-drop: descreve o que está sendo arrastado.
//
// O JUCE (ListBox::startDragAndDrop) já faz todo o trabalho mecânico do
// arraste. Este método apenas empacota o(s) arquivo(s) da(s) linha(s)
// selecionada(s) em um payload tipado, que o DeckGUI vai decodificar no
// itemDropped().
//
// Retornar var() (void) faz o JUCE cancelar o arraste, então retornamos
// apenas quando há uma faixa válida sob o cursor.
juce::var PlaylistComponent::getDragSourceDescription (const juce::SparseSet<int>& rows)
{
    if (rows.isEmpty())
        return {};

    // Assume a primeira linha do conjunto selecionado.
    if (rows.getNumRanges() <= 0)
        return {};

    const auto row = rows.getRange (0).getStart();

    if (row < 0 || row >= songs.size())
        return {};

    return juce::var (new PlaylistDragPayload (songs.getReference (row).file));
}

//==============================================================================
void PlaylistComponent::loadSongAtRow(int row)
{
    if (row < 0 || row >= songs.size())
        return;

    if (onSongSelected)
        onSongSelected (juce::URL { songs.getReference (row).file });
}

//==============================================================================
void PlaylistComponent::mouseDoubleClick (const juce::MouseEvent& event)
{
    // Descobre em qual linha o duplo-clique aconteceu, usando a geometria
    // da tabela (altura do cabeçalho + altura da linha).
    const auto localPos = playlistTable.getLocalPoint (nullptr, event.getScreenPosition());
    const auto headerHeight = playlistTable.getHeaderHeight();
    const auto rowHeight = playlistTable.getRowHeight();

    if (rowHeight <= 0)
        return;

    const int row = (localPos.y - headerHeight) / rowHeight;

    if (row >= 0 && row < songs.size())
    {
        // Seleciona a linha para dar feedback visual e carrega no deck 1.
        playlistTable.selectRow (row, true, true);
        selectedRow = row;
        loadSongAtRow (row);
    }
}

//==============================================================================
const Song* PlaylistComponent::getSelectedSong() const
{
    // ATENÇÃO (JUCE 9): juce::Array::getUnchecked() e operator[] retornam o
    // elemento POR VALOR. Pegar o endereço de um deles devolveria o endereço de
    // um temporário (dangling pointer). getReference() devolve a referência real.
    if (selectedRow >= 0 && selectedRow < songs.size())
        return &songs.getReference(selectedRow);

    return nullptr;
}
