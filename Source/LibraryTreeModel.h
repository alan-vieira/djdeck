#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// No da arvore de navegacao da biblioteca.
//
// Deriva de juce::TreeViewItem. NOTA: o metodo de desenho chama-se
// `paintItem`, nao `paintTreeItem`.
//
// O TreeView NAO tem posse do root — quem o cria precisa guarda-lo como
// member.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §8.2.
class LibraryTreeModel : public juce::TreeViewItem
{
public:
    enum class NodeType
    {
        collection,
        playlists,
        explorer,
        devices,
        histories,
        recordings,
        folder
    };

    LibraryTreeModel (const juce::String& nameToUse, NodeType typeToUse);
    ~LibraryTreeModel() override;

    void paintItem (juce::Graphics& g, int width, int height) override;
    void itemClicked (const juce::MouseEvent& event) override;
    void itemDoubleClicked (const juce::MouseEvent& event) override;

    // TreeViewItem exige esta implementacao: sem ela a classe fica
    // abstrata e nao pode ser instanciada. NAO e const — a assinatura da
    // base e `virtual bool mightContainSubItems() = 0`.
    bool mightContainSubItems() override { return true; }

    juce::String getTitle() const { return title; }
    NodeType getNodeType() const { return nodeType; }

    // Notificado quando o usuario clica num no.
    std::function<void (LibraryTreeModel*)> onSelected;

private:
    juce::String title;
    NodeType nodeType;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibraryTreeModel)
};
