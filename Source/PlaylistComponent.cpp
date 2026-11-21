#include "PlaylistComponent.h"
#include "LibraryStore.h"
#include "LayoutConfig.hpp"
#include "TagLibReader.h"

#include "BpmDetector.h"

#include <functional>

// IDs das colunas da tabela
enum PlaylistColumns
{
    infoColumn = 1,      // Titulo + artista
    albumColumn = 2,
    genreColumn = 3,
    ratingColumn = 4,
    lengthColumn = 5     // Duracao
};

//==============================================================================
PlaylistComponent::PlaylistComponent (juce::AudioFormatManager& formatManagerToUse)
    : formatManager (formatManagerToUse)
{
    addAndMakeVisible(playlistTable);
    playlistTable.setModel(this);

    // Colunas da biblioteca (docs/DOCUMENTACAO_LAYOUT_UI.md §8.3).
    playlistTable.getHeader().addColumn(u8"Título / Artista", infoColumn, 300);
    playlistTable.getHeader().addColumn(u8"Álbum", albumColumn, 180);
    playlistTable.getHeader().addColumn(u8"Gênero", genreColumn, 150);
    playlistTable.getHeader().addColumn(u8"Rating", ratingColumn, 90);
    playlistTable.getHeader().addColumn(u8"Duração", lengthColumn, 80);

    addAndMakeVisible(addButton);
    addButton.addListener(this);
    addButton.setTooltip("Abrir arquivo(s) de áudio");

    // Detecta o duplo-clique nas linhas da tabela. A TableListBox não expõe
    // um callback para isso, então usamos um MouseListener registrado nela.
    // wantsEventsForAllNestedChildComponents = true porque as linhas são
    // componentes filhos internos da ListBox.
    playlistTable.addMouseListener (this, true);

    // Carrega a biblioteca persistida do disco.
    try
    {
        loadLibrary();
    }
    catch (...)
    {
        // Se a leitura da biblioteca falhar, iniciamos com biblioteca vazia.
        // O usuario pode reimportar as faixas pelo botao "Adicionar faixa".
    }
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

    // Relê os metadados do arquivo: uma edição feita fora do app (ou por
    // outro programa de DJ) aparece na próxima vez que o djdeck abre.
    // Faixas sem BPM na tag sao encaminhadas para a deteccao por analise de
    // audio (roda em segundo plano, uma por vez).
    for (int i = 0; i < songs.size(); ++i)
    {
        auto& song = songs.getReference (i);
        readMetadataFor (song);

        if (song.bpm <= 0.0)
            detectBpmFor (i);
    }

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
    // Fundo da area de faixas: #1A1A1A (mesmo do ".track-table" do
    // prototipo HTML e do token "Panel" do layout.json).
    g.fillAll(djdeck::layout::colours::Panel);
}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds();

    // O prototipo HTML nao tem um botao "Adicionar faixa" no topo da lista:
    // a tabela comeca imediatamente com o cabecalho de colunas. O botao do
    // JUCE e mantido (funcao de importar faixas), mas flutua no rodape em
    // vez de empurrar a tabela 40 px para baixo.
    playlistTable.setBounds(area);

    constexpr int buttonWidth = 120;
    constexpr int buttonHeight = 26;
    constexpr int margin = 8;

    addButton.setBounds(area.getRight() - buttonWidth - margin,
                        area.getBottom() - buttonHeight - margin,
                        buttonWidth,
                        buttonHeight);
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
    g.setColour (djdeck::layout::colours::Primary);

    switch (columnId)
{
    case infoColumn:
        g.setColour (djdeck::layout::colours::Primary);
        g.drawText (song.title, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        g.setColour (djdeck::layout::colours::Secondary);
        g.drawText (song.artist, 2, 15, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case albumColumn:
        g.setColour (djdeck::layout::colours::Secondary);
        g.drawText (song.album, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case genreColumn:
        g.setColour (djdeck::layout::colours::Tertiary);
        g.drawText (song.genre, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        break;

    case ratingColumn:
        drawRating (g, song.rating, juce::Rectangle<int> (2, 0, width - 4, height));
        break;

    case lengthColumn:
        g.setColour (djdeck::layout::colours::Secondary);
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

        g.setColour (i < rating ? djdeck::layout::colours::Rating : djdeck::layout::colours::Disabled);
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

                                        // Se o arquivo nao tem BPM na tag,
                                        // detecta por analise de audio em
                                        // segundo plano.
                                        if (song.bpm <= 0.0)
                                            detectBpmFor (songs.size() - 1);
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

        // Se o arquivo nao tem BPM na tag, detecta por analise de audio.
        if (song.bpm <= 0.0)
            detectBpmFor (songs.size() - 1);
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

    const auto& song = songs.getReference (row);
    return juce::var (new PlaylistDragPayload (song.file, song.bpm, song.title, song.artist, song.key));
}

//==============================================================================
void PlaylistComponent::loadSongAtRow(int row)
{
    if (row < 0 || row >= songs.size())
        return;

    const auto& song = songs.getReference (row);

    // O duplo-clique carrega no Deck 1. Passamos também o BPM, título, artista
    // e tom lidos pelo TagLib, para o deck exibir metadados reais no cabeçalho.
    if (onSongSelected)
        onSongSelected (juce::URL { song.file }, song.bpm, song.title, song.artist, song.key, 1);
}

//==============================================================================
void PlaylistComponent::mouseDoubleClick (const juce::MouseEvent& event)
{
    // Forma segura e nativa do JUCE de obter a linha sob o mouse,
    // convertendo coordenadas de tela para locais da tabela.
    const auto localPos = playlistTable.getLocalPoint (nullptr, event.getScreenPosition());
    const int row = playlistTable.getRowContainingPosition (localPos.x, localPos.y);

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

//==============================================================================
// Job de deteccao de BPM: roda no ThreadPool (uma faixa por vez) e avisa o
// resultado na message thread para atualizar o modelo da tabela.
namespace
{
    class BpmDetectionJob : public juce::ThreadPoolJob
    {
    public:
        BpmDetectionJob (juce::File fileToScan,
                         juce::AudioFormatManager& formatManagerToUse,
                         std::function<void (double)> resultCallback)
            : juce::ThreadPoolJob ("BPM Detection"),
              file (std::move (fileToScan)),
              formatManager (formatManagerToUse),
              callback (std::move (resultCallback))
        {
        }

        JobStatus runJob() override
        {
            std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
            double bpm = 0.0;

            if (reader != nullptr)
                bpm = BpmDetector::estimate (*reader, nullptr);

            // Avisa na thread de GUI: so la e seguro tocar no modelo da tabela.
            juce::MessageManager::callAsync ([cb = std::move (callback), bpm] () mutable
            {
                if (cb)
                    cb (bpm);
            });

            return jobHasFinished;
        }

    private:
        juce::File file;
        juce::AudioFormatManager& formatManager;
        std::function<void (double)> callback;
    };
}

void PlaylistComponent::detectBpmFor (int row)
{
    const auto file = songs.getReference (row).file;

    bpmPool.addJob (new BpmDetectionJob (file, formatManager,
        [this, row] (double bpm)
        {
            if (row >= 0 && row < songs.size() && bpm > 0.0)
            {
                songs.getReference (row).bpm = bpm;
                playlistTable.updateContent();
            }
        }), true);
}
