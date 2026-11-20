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
    // Divisao de largura EXATA do Figma (no "arvore": x=0, w=249; no
    // "biblioteca de faixas": x=249, w=1203) — e a mesma do prototipo HTML
    // (.folder-tree { width: 249px; flex-shrink: 0 }).
    //
    // Antes o StretchableLayoutManager recebia (min=100, max=400,
    // preferred=0.25): o "0.25" e um valor ABSOLUTO em pixels (proporcao
    // exige sinal negativo), entao a arvore colapsava no minimo de 100 px
    // e a lista comecava em x=100, 149 px antes da posicao do Figma.
    const auto treeWidth = juce::jmin (djdeck::layout::figmaNodelibraryTree::widthPx, getWidth());

    treeView.setBounds (0, 0, treeWidth, getHeight());
    trackList->setBounds (treeWidth, 0, getWidth() - treeWidth, getHeight());
}
