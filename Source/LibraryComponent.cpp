#include "LibraryComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
LibraryComponent::LibraryComponent (juce::AudioFormatManager& formatManagerToUse)
    : formatManager (formatManagerToUse),
      trackList (std::make_unique<PlaylistComponent> (formatManager))
{
    addAndMakeVisible (treeView);
    addAndMakeVisible (*trackList);

    treeView.setRootItemVisible (true);
    treeView.setIndentSize (14);

    buildTree();
}

LibraryComponent::~LibraryComponent() = default;

//==============================================================================
void LibraryComponent::buildTree()
{
    auto root = std::make_unique<LibraryTreeModel> ("Collection", LibraryTreeModel::NodeType::collection);

    // Playlists — expansivel.
    auto playlists = std::make_unique<LibraryTreeModel> ("Playlists", LibraryTreeModel::NodeType::playlists);
    playlists->addSubItem (std::make_unique<LibraryTreeModel> ("Favorites", LibraryTreeModel::NodeType::folder).release());
    playlists->addSubItem (std::make_unique<LibraryTreeModel> ("Imported", LibraryTreeModel::NodeType::folder).release());
    root->addSubItem (playlists.release());

    // Explorer — expansivel.
    auto explorer = std::make_unique<LibraryTreeModel> ("Explorer", LibraryTreeModel::NodeType::explorer);
    explorer->addSubItem (std::make_unique<LibraryTreeModel> ("Music", LibraryTreeModel::NodeType::folder).release());
    root->addSubItem (explorer.release());

    root->addSubItem (std::make_unique<LibraryTreeModel> ("Devices", LibraryTreeModel::NodeType::devices).release());
    root->addSubItem (std::make_unique<LibraryTreeModel> ("Histories", LibraryTreeModel::NodeType::histories).release());
    root->addSubItem (std::make_unique<LibraryTreeModel> ("Recordings", LibraryTreeModel::NodeType::recordings).release());

    treeView.setRootItem (root.get());

    // O TreeView nao tem posse do root — guardamos como member.
    treeRoot = std::move (root);
}

//==============================================================================
void LibraryComponent::paint (juce::Graphics& g)
{
    g.fillAll (djdeck::layout::colours::Background);
}

//==============================================================================
void LibraryComponent::resized()
{
    // juce::SplitterComponent nao existe no JUCE 9. O mecanismo equivalente e
    // o StretchableLayoutManager.
    //
    // Numeros negativos em setItemLayout sao proporcoes do espaco disponivel.
    layoutManager.setItemLayout (0, 100, 400, 0.25);   // arvore (frame1) — 25%
    layoutManager.setItemLayout (1, 200, -0.5, -0.75);  // lista de faixas (frame2) — 75%

    Component* components[] = { &treeView, trackList.get() };

    // vertically = false -> os componentes sao distribuidos ao longo do eixo X,
    // cada um recebendo a sua largura. resizeOtherDimension = true -> tambem
    // preenchem a altura toda.
    layoutManager.layOutComponents (components, 2,
                                   0, 0, getWidth(), getHeight(),
                                   false, true);
}
