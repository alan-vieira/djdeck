#include "LibraryComponent.h"
#include "RekordboxColours.h"

//==============================================================================
LibraryComponent::LibraryComponent()
    : trackList (std::make_unique<PlaylistComponent>())
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
    g.fillAll (RekordboxColours::background);
}

//==============================================================================
void LibraryComponent::resized()
{
    // juce::SplitterComponent nao existe no JUCE 9. O mecanismo equivalente e
    // o StretchableLayoutManager.
    //
    // Numeros negativos em setItemLayout sao proporcoes do espaco disponivel.
    layoutManager.setItemLayout (0, 120, 600, 0.30);   // arvore
    layoutManager.setItemLayout (1, 200, -0.5, -0.7);  // lista de faixas

    Component* components[] = { &treeView, trackList.get() };

    // vertically = false -> os componentes sao distribuidos ao longo do eixo X,
    // cada um recebendo a sua largura. resizeOtherDimension = true -> tambem
    // preenchem a altura toda.
    layoutManager.layOutComponents (components, 2,
                                   0, 0, getWidth(), getHeight(),
                                   false, true);
}
