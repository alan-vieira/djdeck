#include "LibraryTreeModel.h"
#include "RekordboxColours.h"

//==============================================================================
LibraryTreeModel::LibraryTreeModel (const juce::String& nameToUse, NodeType typeToUse)
    : title (nameToUse),
      nodeType (typeToUse)
{
}

LibraryTreeModel::~LibraryTreeModel() = default;

//==============================================================================
void LibraryTreeModel::paintItem (juce::Graphics& g, int width, int height)
{
    // TreeViewItem NAO e um Component: nao tem getLocalBounds(). A area vem
    // nos parametros da propria funcao.
    juce::Rectangle<float> area (0.0f, 0.0f,
                                 static_cast<float> (width),
                                 static_cast<float> (height));

    // Icone, desenhado como um quadrado colorido com o tipo do no.
    constexpr float iconSize = 10.0f;
    const auto iconY = (area.getHeight() - iconSize) * 0.5f;

    juce::Colour iconColour = RekordboxColours::textTertiary;

    switch (nodeType)
    {
        case NodeType::collection: iconColour = RekordboxColours::accent;       break;
        case NodeType::playlists:  iconColour = RekordboxColours::loop;         break;
        case NodeType::explorer:   iconColour = RekordboxColours::textTertiary; break;
        case NodeType::devices:    iconColour = RekordboxColours::textTertiary; break;
        case NodeType::histories:  iconColour = RekordboxColours::textTertiary; break;
        case NodeType::recordings: iconColour = RekordboxColours::cue;          break;
        case NodeType::folder:     iconColour = RekordboxColours::textTertiary; break;
    }

    g.setColour (iconColour);
    g.fillRoundedRectangle (area.getX(), iconY, iconSize, iconSize, 2.0f);

    // Texto do no.
    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.setColour (isSelected() ? RekordboxColours::textPrimary : RekordboxColours::textSecondary);

    // removeFromLeft devolve um Rectangle<float>; drawText aceita essa forma.
    g.drawText (title,
                area.removeFromLeft (iconSize + 6.0f).withTrimmedRight (4.0f),
                juce::Justification::centredLeft, true);
}

//==============================================================================
void LibraryTreeModel::itemClicked (const juce::MouseEvent&)
{
    if (onSelected != nullptr)
        onSelected (this);
}

void LibraryTreeModel::itemDoubleClicked (const juce::MouseEvent&)
{
    // Duplo-clique expande o no — comportamento padrao do TreeViewItem.
}
