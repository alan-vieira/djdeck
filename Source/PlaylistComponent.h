#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "Song.h"
#include "PlaylistDragPayload.h"
#include "TagLibReader.h"

// Componente de biblioteca/playlist.
// Exibe uma TableListBox de faixas importadas e permite arrastar uma faixa
// para um deck (drag-and-drop interno do app).
//
// NOTA: juce::Component já herda de juce::MouseListener, por isso não
// declaramos essa base aqui de novo (o compilador emitiria C4584).
class PlaylistComponent : public juce::Component,
                          public juce::TableListBoxModel,
                          public juce::Button::Listener,
                          public juce::FileDragAndDropTarget
{
public:
    // O formatManager e usado para decodificar o audio na deteccao de BPM por
    // analise, que roda na thread de fundo (nunca na thread de GUI).
    PlaylistComponent (juce::AudioFormatManager& formatManagerToUse);
    ~PlaylistComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // TableListBoxModel
    int getNumRows() override;
    void paintRowBackground(juce::Graphics&, int rowNumber, int width, int height,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics&, int rowNumber, int columnId, int width, int height,
                   bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId,
                                             bool rowIsSelected,
                                             juce::Component* existingComponentToUpdate) override;
    void selectedRowsChanged(int lastRowSelected) override;
    void sortOrderChanged(int newSortColumnId, bool isForwards) override;

    // TableListBoxModel - drag-and-drop.
    // O JUCE já implementa o arraste nativamente na ListBox: basta devolver
    // aqui a descrição do que está sendo arrastado. O ListBox cuida do
    // snapshot, do "ghost" e do rastreio do mouse; o DragAndDropContainer
    // (MainComponent) entrega os eventos de drop nos decks.
    juce::var getDragSourceDescription (const juce::SparseSet<int>& rows) override;
    bool mayDragToExternalWindows() const override { return false; }

    // Button::Listener
    void buttonClicked(juce::Button* button) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override;

    // Retorna a faixa atualmente selecionada (ou nenhuma)
    const Song* getSelectedSong() const;

    // Callback chamado quando o usuário seleciona uma faixa para carregar em um deck.
    // Parâmetros: URL, BPM, título, artista, tom musical e ID do deck alvo (1 = deck 1, padrão).
    std::function<void(juce::URL, double, const juce::String&, const juce::String&,
                       const juce::String&, int)> onSongSelected;

    // Carrega a faixa de um índice (usado no duplo-clique)
    void loadSongAtRow(int row);

    // MouseListener - duplo-clique numa linha carrega a faixa no deck 1
    void mouseDoubleClick (const juce::MouseEvent& event) override;

private:
    juce::AudioFormatManager& formatManager;

    // Detecta BPM de uma unica faixa por vez (nao empilha threads por faixa
    // importada).
    juce::ThreadPool bpmPool { 1 };

    // Rating: 5 estrelas preenchidas, em ambar.
    void drawRating (juce::Graphics& g, int rating, const juce::Rectangle<int>& area);

    // Le os metadados do arquivo com o TagLib e aplica na faixa.
    void readMetadataFor (Song& song);

    // Deteccao de BPM por analise de audio da faixa na linha `row`; atualiza a
    // linha da tabela quando termina.
    void detectBpmFor (int row);

    // Carrega a biblioteca persistida do disco.
    void loadLibrary();

    // Grava a biblioteca no disco.
    void saveLibrary() const;

    juce::TableListBox playlistTable;
    juce::TextButton addButton{ "Adicionar faixa" };

    juce::Array<Song> songs;
    int selectedRow = -1;

    // Leitor de metadados (TagLib). Compartilhado entre as faixas.
    TagLibReader metadataReader;

    juce::FileChooser fileChooser{ "Selecione arquivos de áudio",
                                   juce::File(),
                                   "*.wav;*.aiff;*.flac;*.ogg;*.mp3" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
