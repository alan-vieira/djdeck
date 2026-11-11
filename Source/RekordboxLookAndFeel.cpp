#include "RekordboxLookAndFeel.h"

//==============================================================================
RekordboxLookAndFeel::RekordboxLookAndFeel()
{
    // Sliders
    setColour (juce::Slider::backgroundColourId,         RekordboxColours::canvas);
    setColour (juce::Slider::trackColourId,               RekordboxColours::separator);
    setColour (juce::Slider::thumbColourId,                RekordboxColours::textSecondary);
    setColour (juce::Slider::rotarySliderFillColourId,    RekordboxColours::accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, RekordboxColours::separator);
    setColour (juce::Slider::textBoxTextColourId,         RekordboxColours::textPrimary);
    setColour (juce::Slider::textBoxBackgroundColourId,   RekordboxColours::panelRaised);
    setColour (juce::Slider::textBoxHighlightColourId,    RekordboxColours::accent);
    setColour (juce::Slider::textBoxOutlineColourId,      RekordboxColours::separator);

    // Botoes
    setColour (juce::TextButton::buttonColourId,      RekordboxColours::panelRaised);
    setColour (juce::TextButton::buttonOnColourId,    RekordboxColours::accent);
    setColour (juce::TextButton::textColourOffId,     RekordboxColours::textSecondary);
    setColour (juce::TextButton::textColourOnId,      RekordboxColours::textPrimary);

    // Labels
    setColour (juce::Label::textColourId,      RekordboxColours::textSecondary);
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    // Listas / scrollbars
    setColour (juce::ListBox::backgroundColourId,      RekordboxColours::panel);
    setColour (juce::ListBox::outlineColourId,         RekordboxColours::separator);
    setColour (juce::ListBox::textColourId,            RekordboxColours::textPrimary);
    setColour (juce::ScrollBar::thumbColourId,         RekordboxColours::separator);
    setColour (juce::ScrollBar::trackColourId,         RekordboxColours::canvas);

    // Tabelas — TableListBox nao tem ColourIds proprios; o fundo vem do
    // LookAndFeel da ListBox que ele contem, e o header tem IDs proprios.
    setColour (juce::TableHeaderComponent::backgroundColourId,     RekordboxColours::panelRaised);
    setColour (juce::TableHeaderComponent::outlineColourId,        RekordboxColours::separator);
    setColour (juce::TableHeaderComponent::textColourId,           RekordboxColours::textSecondary);

    // Arvore
    setColour (juce::TreeView::backgroundColourId, RekordboxColours::panel);
    setColour (juce::TreeView::linesColourId,      RekordboxColours::separator);

    // Janelas / popups
    setColour (juce::ResizableWindow::backgroundColourId, RekordboxColours::background);
    setColour (juce::PopupMenu::backgroundColourId,        RekordboxColours::panelRaised);
    setColour (juce::PopupMenu::textColourId,              RekordboxColours::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, RekordboxColours::selection);
    setColour (juce::PopupMenu::highlightedTextColourId,   RekordboxColours::textPrimary);
}

//==============================================================================
void RekordboxLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const auto isVertical = style == juce::Slider::LinearVertical;

    // A area util e a do slider menos a folga de 4 px usada no handle.
    const auto bounds = juce::Rectangle<float> (static_cast<float> (x),
                                                static_cast<float> (y),
                                                static_cast<float> (width),
                                                static_cast<float> (height)).reduced (4.0f);

    if (isVertical)
    {
        const auto centreX = bounds.getCentreX();

        // Trilho
        g.setColour (RekordboxColours::canvas);
        g.fillRoundedRectangle (centreX - 2.0f, bounds.getY(), 4.0f, bounds.getHeight(), 2.0f);

        // Preenchimento: do handle ate o fundo.
        const auto handleY = bounds.getY() + bounds.getHeight() * (1.0f - sliderPos);

        g.setColour (RekordboxColours::textDisabled);
        g.fillRoundedRectangle (centreX - 2.0f, handleY, 4.0f,
                                bounds.getBottom() - handleY, 2.0f);

        // Handle
        auto handle = juce::Rectangle<float> (centreX - 12.0f, handleY - 9.0f, 24.0f, 18.0f);

        g.setColour (RekordboxColours::textSecondary);
        g.fillRoundedRectangle (handle, 3.0f);

        g.setColour (RekordboxColours::canvas);
        g.fillRect (handle.getCentreX() - 1.0f, handle.getY(), 2.0f, handle.getHeight());
    }
    else
    {
        const auto centreY = bounds.getCentreY();

        // Trilho
        g.setColour (RekordboxColours::canvas);
        g.fillRoundedRectangle (bounds.getX(), centreY - 2.0f, bounds.getWidth(), 4.0f, 2.0f);

        // Preenchimento: do inicio ate o handle.
        const auto handleX = bounds.getX() + bounds.getWidth() * sliderPos;

        g.setColour (RekordboxColours::textDisabled);
        g.fillRoundedRectangle (bounds.getX(), centreY - 2.0f, handleX - bounds.getX(), 4.0f, 2.0f);

        // Handle
        auto handle = juce::Rectangle<float> (handleX - 9.0f, centreY - 12.0f, 18.0f, 24.0f);

        g.setColour (RekordboxColours::textSecondary);
        g.fillRoundedRectangle (handle, 3.0f);

        g.setColour (RekordboxColours::canvas);
        g.fillRect (handle.getX(), handle.getCentreY() - 1.0f, handle.getWidth(), 2.0f);
    }
}

//==============================================================================
void RekordboxLookAndFeel::drawKnob (juce::Graphics& g, juce::Rectangle<float> area,
                                     float proportion, float startAngle, float endAngle,
                                     juce::Colour fillColour)
{
    const auto radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 2.0f;

    if (radius <= 2.0f)
        return;

    const auto centre = area.getCentre();

    // Corpo
    g.setColour (RekordboxColours::panelRaised);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (RekordboxColours::separator);
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    // Arco de valor: do inicio da faixa ate a posicao atual.
    // addCentredArc usa RADIANOS, com 0 no topo e sentido horario — o mesmo
    // sistema do setRotaryParameters() do Slider.
    {
        const auto angle = startAngle + proportion * (endAngle - startAngle);

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, radius - 1.0f, radius - 1.0f,
                                0.0f, startAngle, angle, true);

        g.setColour (fillColour);
        g.strokePath (valueArc, juce::PathStrokeType (2.0f));
    }

    // Indicador
    const auto angle = startAngle + proportion * (endAngle - startAngle);
    const auto indicatorLength = radius * 0.7f;

    const auto indicatorX = centre.x + std::sin (angle) * indicatorLength;
    const auto indicatorY = centre.y - std::cos (angle) * indicatorLength;

    g.setColour (RekordboxColours::textPrimary);
    g.drawLine (centre.x, centre.y, indicatorX, indicatorY, 2.0f);
}

//==============================================================================
void RekordboxLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider&)
{
    auto area = juce::Rectangle<float> (static_cast<float> (x),
                                        static_cast<float> (y),
                                        static_cast<float> (width),
                                        static_cast<float> (height));

    // O rotativo nao deve ser quadrado: usamos o menor lado.
    const auto side = static_cast<float> (juce::jmin (width, height));
    area = area.withSizeKeepingCentre (side, side);

    drawKnob (g, area,
              juce::jlimit (0.0f, 1.0f, sliderPosProportional),
              rotaryStartAngle, rotaryEndAngle,
              RekordboxColours::accent);
}

//==============================================================================
int RekordboxLookAndFeel::getSliderThumbRadius (juce::Slider& slider)
{
    if (slider.isRotary())
        return 0;   // o knob e desenhado inteiro por drawRotarySlider

    return 12;
}

//==============================================================================
void RekordboxLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour& backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto area = button.getLocalBounds().toFloat().reduced (1.0f);
    constexpr float cornerSize = 3.0f;

    juce::Colour fill = backgroundColour;

    if (button.getToggleState())
        fill = RekordboxColours::accent;
    else if (shouldDrawButtonAsDown)
        fill = RekordboxColours::accent;
    else if (shouldDrawButtonAsHighlighted)
        fill = RekordboxColours::hover;

    if (! button.isEnabled())
        fill = fill.withMultipliedAlpha (0.5f);

    g.setColour (fill);
    g.fillRoundedRectangle (area, cornerSize);

    g.setColour (RekordboxColours::separator);
    g.drawRoundedRectangle (area, cornerSize, 1.0f);
}

//==============================================================================
void RekordboxLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                           bool isMouseOverButton, bool /*isButtonDown*/)
{
    g.setFont (getTextButtonFont (button, button.getHeight()));

    auto colour = button.findColour (button.getToggleState()
                                        ? juce::TextButton::textColourOnId
                                        : juce::TextButton::textColourOffId);

    if (! button.isEnabled())
        colour = colour.withMultipliedAlpha (0.5f);
    else if (isMouseOverButton)
        colour = RekordboxColours::textPrimary;

    g.setColour (colour);
    g.drawFittedText (button.getButtonText(), 2, 2,
                      button.getWidth() - 4, button.getHeight() - 4,
                      juce::Justification::centred, 2);
}

//==============================================================================
juce::Font RekordboxLookAndFeel::getTextButtonFont (juce::TextButton& button, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jmin (13.0f,
                                                       static_cast<float> (buttonHeight) * 0.5f)));
}

//==============================================================================
juce::Font RekordboxLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions (13.0f));
}

//==============================================================================
juce::Font RekordboxLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (13.0f));
}
