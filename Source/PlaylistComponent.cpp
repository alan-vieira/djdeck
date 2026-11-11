#include "PlaylistComponent.h"
#include "LibraryStore.h"
#include "RekordboxColours.h"
#include "TagLibReader.h"

// IDs das colunas da tabela
enum PlaylistColumns
{
    titleColumn = 1,
    artistColumn,
    lengthColumn,
    bpmColumn,
    keyColumn,
    albumColumn,
    genreColumn,
    ratingColumn
};

//==============================================================================
PlaylistComponent::PlaylistComponent()
{
    addAndMakeVisible(playlistTable);
    playlistTable.setModel(this);

    // Colunas da biblioteca (docs/DOCUMENTACAO_LAYOUT_UI.md §8.3).
    playlistTable.getHeader().addColumn("Título", titleColumn, 200);
    playlistTable.getHeader().addColumn("Artista", artistColumn, 150);
    playlistTable.getHeader().addColumn("BPM", bpmColumn, 70);
    playlistTable.getHeader().addColumn("Key", keyColumn, 60);
    playlistTable.getHeader().addColumn("Álbum", albumColumn, 150);
    playlistTable.getHeader().addColumn("Gênero", genreColumn, 100);
    playlistTable.getHeader().addColumn("Rating", ratingColumn, 90);
    playlistTable.getHeader().addColumn("Duração", lengthColumn, 70);

    addAndMakeVisible(addButton);
    addButton.addListener(this);
    addButton.setTooltip("Abrir arquivo(s) de áudio");

    // Detecta o duplo-clique nas linhas da tabela. A TableListBox não expõe
    // um callback para isso, então usamos um MouseListener registrado nela.
    // wantsEventsForAllNestedChildComponents = true porque as linhas são
    // componentes filhos internos da ListBox.
    playlistTable.addMouseListener (this, true);

    // Carrega a biblioteca persistida do disco.
    loadLibrary();
}

PlaylistComponent::~PlaylistComponent()
{
    // A biblioteca e persistida ao fechar o app.
    saveLibrary();

    playlistTable.setModel(nullptr);
}

//==============================================================================
void PlaylistComponent::loadLibrary()
{
    const auto stored = LibraryStore::load();

    if (stored.isEmpty())
        return;

    songs = stored;

    // Relê os metadados do arquivo: uma edicao feita fora do app (ou por
    // outro programa de DJ) aparece na proxima vez que o djdeck abre.
    for (auto& song : songs)
        readMetadataFor (song);

    playlistTable.updateContent();
}

//==============================================================================
void PlaylistComponent::saveLibrary() const
{
    LibraryStore::save (songs);
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
                                  int width, int height, bool /*rowIsSelected*/)
{
    if (rowNumber < 0 || rowNumber >= songs.size())
        return;

    const auto& song = songs.getReference(rowNumber);

    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.setColour (RekordboxColours::textPrimary);

    switch (columnId)
    {
    case titleColumn:
        g.drawText (song.title, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case artistColumn:
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (song.artist, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case bpmColumn:
    {
        // "140.00" — ou "--" quando o BPM nao foi detectado.
        const auto bpmText = song.bpm > 0.0 ? juce::String (song.bpm, 2) : juce::String ("--");
        g.drawText (bpmText, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;
    }

    case keyColumn:
    {
        // A key vem do arquivo em Camelot ("8A") ou em notacao classica.
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (song.key.isEmpty() ? juce::String ("--") : song.key,
                    2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;
    }

    case albumColumn:
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (song.album, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case genreColumn:
        g.setColour (RekordboxColours::textTertiary);
        g.drawText (song.genre, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case ratingColumn:
        drawRating (g, song.rating, juce::Rectangle<int> (2, 0, width - 4, height));
        break;

    case lengthColumn:
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (song.getFormattedLength(), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    default: break;
    }
}

//==============================================================================
// Rating: 5 estrelas, preenchidas em ambar (#DEAB38).
void PlaylistComponent::drawRating (juce::Graphics& g, int rating,
                                    const juce::Rectangle<int>& area)
{
    if (rating <= 0)
        return;

    constexpr int numStars = 5;
    const auto starWidth = area.getWidth() / numStars;
    const auto centreY = static_cast<float> (area.getCentreY());

    for (int i = 0; i < numStars; ++i)
    {
        const auto x = static_cast<float> (area.getX()) + i * starWidth + starWidth * 0.5f;
        const auto radius = juce::jmin (5.0f, static_cast<float> (starWidth) * 0.4f);

        // Estrela de 5 pontas, desenhada como um poligono.
        juce::Path star;
        constexpr int points = 5;

        for (int p = 0; p < points * 2; ++p)
        {
            const auto angle = juce::MathConstants<float>::halfPi
                             + static_cast<float> (p) * juce::MathConstants<float>::pi / static_cast<float> (points);
            const auto r = (p % 2 == 0) ? radius : radius * 0.45f;

            const auto px = x + std::cos (angle) * r;
            const auto py = centreY - std::sin (angle) * r;

            if (p == 0)
                star.startNewSubPath (px, py);
            else
                star.lineTo (px, py);
        }

        star.closeSubPath();

        g.setColour (i < rating ? RekordboxColours::rating : RekordboxColours::textDisabled);
        g.fillPath (star);
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
// Le os metadados do arquivo e aplica na faixa.
//
// O TagLib da titulo, artista, album, genero, BPM, key e rating — sem isso a
// biblioteca mostraria apenas o nome do arquivo.
void PlaylistComponent::readMetadataFor (Song& song)
{
    const auto metadata = metadataReader.readMetadata (song.file);

    if (metadata.isValid)
        song.applyMetadata (metadata);
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
                                        readMetadataFor (song);
                                        songs.add (song);
                                    }
                                    playlistTable.updateContent();

                                    // Persiste a biblioteca: uma queda do app nao
                                    // deve custar a lista montada.
                                    saveLibrary();
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
        readMetadataFor (song);
        songs.add(song);
    }
    playlistTable.updateContent();

    // Persiste a biblioteca.
    saveLibrary();
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

    const auto& song = songs.getReference (row);

    // O duplo-clique carrega no Deck 1. Passamos tambem o BPM lido pelo
    // TagLib, para o deck exibir no jog wheel e calcular o pitch correto.
    if (onSongSelected)
        onSongSelected (juce::URL { song.file }, song.bpm);
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
