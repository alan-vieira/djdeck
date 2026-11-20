#include "RekordboxLookAndFeel.h"
#include "LayoutConfig.hpp"

//==============================================================================
RekordboxLookAndFeel::RekordboxLookAndFeel()
{
    // Sliders
    setColour (juce::Slider::backgroundColourId,         djdeck::layout::colours::Canvas);
    setColour (juce::Slider::trackColourId,               djdeck::layout::colours::Separator);
    setColour (juce::Slider::thumbColourId,                djdeck::layout::colours::Secondary);
    setColour (juce::Slider::rotarySliderFillColourId,    djdeck::layout::colours::Accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, djdeck::layout::colours::Separator);
    setColour (juce::Slider::textBoxTextColourId,         djdeck::layout::colours::Primary);
    setColour (juce::Slider::textBoxBackgroundColourId,   djdeck::layout::colours::Panelraised);
    setColour (juce::Slider::textBoxHighlightColourId,    djdeck::layout::colours::Accent);
    setColour (juce::Slider::textBoxOutlineColourId,      djdeck::layout::colours::Separator);

    // Botoes
    setColour (juce::TextButton::buttonColourId,      djdeck::layout::colours::Panelraised);
    setColour (juce::TextButton::buttonOnColourId,    djdeck::layout::colours::Accent);
    setColour (juce::TextButton::textColourOffId,     djdeck::layout::colours::Secondary);
    setColour (juce::TextButton::textColourOnId,      djdeck::layout::colours::Primary);

    // Labels
    setColour (juce::Label::textColourId,      djdeck::layout::colours::Secondary);
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    // Listas / scrollbars
    setColour (juce::ListBox::backgroundColourId,      djdeck::layout::colours::Panel);
    setColour (juce::ListBox::outlineColourId,         djdeck::layout::colours::Separator);
    setColour (juce::ListBox::textColourId,            djdeck::layout::colours::Primary);
    setColour (juce::ScrollBar::thumbColourId,         djdeck::layout::colours::Separator);
    setColour (juce::ScrollBar::trackColourId,         djdeck::layout::colours::Canvas);

    // Tabelas — TableListBox nao tem ColourIds proprios; o fundo vem do
    // LookAndFeel da ListBox que ele contem, e o header tem IDs proprios.
    setColour (juce::TableHeaderComponent::backgroundColourId,     djdeck::layout::colours::Panelraised);
    setColour (juce::TableHeaderComponent::outlineColourId,        djdeck::layout::colours::Separator);
    setColour (juce::TableHeaderComponent::textColourId,           djdeck::layout::colours::Secondary);

    // Arvore
    setColour (juce::TreeView::backgroundColourId, djdeck::layout::colours::Panel);
    setColour (juce::TreeView::linesColourId,      djdeck::layout::colours::Separator);

    // Janelas / popups
    setColour (juce::ResizableWindow::backgroundColourId, djdeck::layout::colours::Background);
    setColour (juce::PopupMenu::backgroundColourId,        djdeck::layout::colours::Panelraised);
    setColour (juce::PopupMenu::textColourId,              djdeck::layout::colours::Primary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, djdeck::layout::colours::Selection);
    setColour (juce::PopupMenu::highlightedTextColourId,   djdeck::layout::colours::Primary);
}

//==============================================================================
void RekordboxLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                             juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const auto isVertical = style == juce::Slider::LinearVertical;
    const auto name = slider.getName();
    const auto isMixerVolume  = name == "mixerVolume";
    const auto isMixerCrossfader = name == "mixerCrossfader";
    const auto isPitch = name == "pitch";

    // x/y/width/height = sliderBounds (regiao percorrida pelo thumb), ja em
    // coords locais do slider. Para sliders do mixer, isso E o proprio trilho.
    const auto bounds = juce::Rectangle<float> (static_cast<float> (x),
                                                static_cast<float> (y),
                                                static_cast<float> (width),
                                                static_cast<float> (height));

    // Geometria do trilho e do handle.
    juce::Rectangle<float> rail, handle;

    if (isMixerVolume)
    {
        // Vertical: trilho = bounds (2 x 46). Handle centrado no centro X do trilho.
        rail = bounds;
        const auto centreX = rail.getCentreX();
        const auto handleY = juce::jlimit (rail.getY(), rail.getBottom(), sliderPos);
        handle = juce::Rectangle<float> (
            centreX - djdeck::layout::figmaNodemixerVolBtn1::widthPx / 2.0f,
            handleY - djdeck::layout::figmaNodemixerVolBtn1::heightPx / 2.0f,
            djdeck::layout::figmaNodemixerVolBtn1::widthPx,
            djdeck::layout::figmaNodemixerVolBtn1::heightPx);
    }
    else if (isMixerCrossfader)
    {
        // Horizontal: trilho = bounds (99 x 2). Handle centrado no centro Y do trilho.
        rail = bounds;
        const auto centreY = rail.getCentreY();
        const auto handleX = juce::jlimit (rail.getX(), rail.getRight(), sliderPos);
        handle = juce::Rectangle<float> (
            handleX - djdeck::layout::figmaNodemixerCrossfaderBtn1::widthPx / 2.0f,
            centreY - djdeck::layout::figmaNodemixerCrossfaderBtn1::heightPx / 2.0f,
            djdeck::layout::figmaNodemixerCrossfaderBtn1::widthPx,
            djdeck::layout::figmaNodemixerCrossfaderBtn1::heightPx);
    }
    else
    {
        // Pitch (e demais sliders nao-mixer): comportamento ORIGINAL do HEAD,
        // preservado byte-a-byte — trilho de 4 px e handle de 24 x 18 no
        // vertical, trilho de 4 px e handle de 18 x 24 no horizontal.
        // Os pinos de referencia do pitch sao dimensionados em funcao do
        // handle de 24 px, portanto nao podem ser reduzidos.
        if (isVertical)
        {
            const auto centreX = bounds.getCentreX();
            const auto handleY = juce::jlimit (bounds.getY(), bounds.getBottom(), sliderPos);
            rail = juce::Rectangle<float> (
                centreX - 2.0f,
                bounds.getY(),
                4.0f,
                bounds.getHeight());
            handle = juce::Rectangle<float> (
                centreX - 12.0f,
                handleY - 9.0f,
                24.0f,
                18.0f);
        }
        else
        {
            const auto centreY = bounds.getCentreY();
            const auto handleX = juce::jlimit (bounds.getX(), bounds.getRight(), sliderPos);
            rail = juce::Rectangle<float> (
                bounds.getX(),
                centreY - 2.0f,
                bounds.getWidth(),
                4.0f);
            handle = juce::Rectangle<float> (
                handleX - 9.0f,
                centreY - 12.0f,
                18.0f,
                24.0f);
        }
    }

    if (isVertical)
    {
        const auto centreX = rail.getCentreX();
        const auto handleY = handle.getCentreY();
        const auto railBottom = rail.getBottom();

        // Trilho
        g.setColour (isMixerVolume ? djdeck::layout::colours::VolumeRail
                                   : djdeck::layout::colours::Canvas);
        g.fillRoundedRectangle (rail, 2.0f);

        // Pinos de referencia do pitch: marcadores horizontais fixos no minimo,
        // no 0% e no maximo da faixa. Desenhados ANTES do preenchimento para
        // que a linha azul de valor atravesse eles sem ser interrompida.
        if (isPitch)
            drawPitchReferencePins (g, bounds, slider);

        // Preenchimento: do handle ate a base (a parte ja percorrida).
        g.setColour (isPitch ? djdeck::layout::colours::Accent
                             : djdeck::layout::colours::Disabled);
        g.fillRoundedRectangle (rail.getX(), handleY, rail.getWidth(),
                                railBottom - handleY, 2.0f);

        // Handle
        g.setColour (isMixerVolume ? djdeck::layout::colours::VolumeButton
                                   : djdeck::layout::colours::Secondary);
        g.fillRoundedRectangle (handle, 3.0f);

        g.setColour (djdeck::layout::colours::Canvas);
        g.fillRect (handle.getCentreX() - 1.0f, handle.getY(), 2.0f, handle.getHeight());

        // Pino de posicao: marcador na trilha indicando onde o slider esta.
        g.setColour (djdeck::layout::colours::Accent);
        g.fillRect (centreX - 6.0f, handleY - 1.5f, 12.0f, 3.0f);
    }
    else
    {
        const auto centreY = rail.getCentreY();
        const auto handleX = handle.getCentreX();

        // Trilho
        g.setColour (djdeck::layout::colours::Canvas);
        g.fillRoundedRectangle (rail, 2.0f);

        // Preenchimento: do inicio ate o handle.
        g.setColour (djdeck::layout::colours::Disabled);
        g.fillRoundedRectangle (bounds.getX(), rail.getY(), handleX - bounds.getX(),
                                rail.getHeight(), 1.0f);

        // Handle
        g.setColour (isMixerCrossfader ? djdeck::layout::colours::VolumeButton
                                       : djdeck::layout::colours::Secondary);
        g.fillRoundedRectangle (handle, 3.0f);

        g.setColour (djdeck::layout::colours::Canvas);
        g.fillRect (handle.getX(), centreY - 1.0f, handle.getWidth(), 2.0f);

        // Pino de posicao
        g.setColour (djdeck::layout::colours::Accent);
        g.fillRect (handleX - 1.5f, centreY - 6.0f, 3.0f, 12.0f);
    }
}

//==============================================================================
juce::Slider::SliderLayout RekordboxLookAndFeel::getSliderLayout (juce::Slider& slider)
{
    const auto name = slider.getName();

    // Sliders nao-mixer: comportamento padrao do JUCE (pitch, etc.).
    if (name != "mixerVolume" && name != "mixerCrossfader")
        return juce::LookAndFeel_V4::getSliderLayout (slider);

    juce::Slider::SliderLayout layout;
    layout.textBoxBounds = juce::Rectangle<int>();  // NoTextBox

    if (name == "mixerVolume")
    {
        // Vertical: trilho de 2 x 46 px, curso vertical completo (46 px).
        // O trilho fica centralizado horizontalmente no componente:
        // centro do trilho = width/2 => sliderBounds.x = width/2 - 1.
        auto w = slider.getWidth();
        auto h = slider.getHeight();
        layout.sliderBounds = juce::Rectangle<int> (w / 2 - 1, 0, 2, h);
    }
    else // mixerCrossfader
    {
        // Horizontal: trilho de 99 x 2 px, curso horizontal completo (99 px).
        // O trilho fica centralizado verticalmente no componente:
        // centro do trilho = height/2 => sliderBounds.y = height/2 - 1.
        auto w = slider.getWidth();
        auto h = slider.getHeight();
        layout.sliderBounds = juce::Rectangle<int> (0, h / 2 - 1, w, 2);
    }

    return layout;
}

//==============================================================================
void RekordboxLookAndFeel::drawPitchReferencePins (juce::Graphics& g,
                                                   juce::Rectangle<float> bounds,
                                                   juce::Slider& slider)
{
    // As posicoes sao derivadas do range REAL do slider (valueToProportionOfLength
    // devolve 0..1 dentro da faixa), e nao de proporcoes fixas: assim o pino
    // central cai no 0% real do pitch, e os extremos nos limites do range.
    const auto minValue = slider.getMinimum();
    const auto maxValue = slider.getMaximum();

    if (minValue >= maxValue)
        return;

    // Tres pinos igualmente espacados: minimo, 0% e maximo da faixa. Num
    // range simetrico (+-16%) o 0% cai no meio do trilho, como na referencia
    // visual do Rekordbox. As posicoes sao derivadas do range REAL do slider
    // via valueToProportionOfLength() — nunca de proporcoes fixas.
    struct Pin { double value; bool isCentre; };
    const Pin pins[] = { { minValue, false }, { 0.0, true }, { maxValue, false } };

    const auto centreX = bounds.getCentreX();

    for (const auto& pin : pins)
    {
        if (pin.value < minValue || pin.value > maxValue)
            continue;

        const auto proportion = juce::jlimit (0.0, 1.0, slider.valueToProportionOfLength (pin.value));

        // Vertical: proporcao 0 = base (minimo), 1 = topo (maximo).
        const auto pinY = juce::jlimit (bounds.getY() + 1.0f,
                                       bounds.getBottom() - 1.0f,
                                       bounds.getBottom() - bounds.getHeight() * static_cast<float> (proportion));

        // O pino central (0%) e o mais destacado: e o ponto de retorno ao
        // velocidade original da faixa. Os pinos sao 4 px mais largos que o
        // handle (24 px), para que continuem visiveis sob ele.
        g.setColour (pin.isCentre ? djdeck::layout::colours::Tertiary : djdeck::layout::colours::Disabled);
        g.fillRect (centreX - 14.0f, pinY - 1.0f, 28.0f, 2.0f);
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
    g.setColour (djdeck::layout::colours::Panelraised);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (djdeck::layout::colours::Separator);
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

    g.setColour (djdeck::layout::colours::Primary);
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
              djdeck::layout::colours::Accent);
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
        fill = djdeck::layout::colours::Accent;
    else if (shouldDrawButtonAsDown)
        fill = djdeck::layout::colours::Accent;
    else if (shouldDrawButtonAsHighlighted)
        fill = djdeck::layout::colours::Hover;

    if (! button.isEnabled())
        fill = fill.withMultipliedAlpha (0.5f);

    g.setColour (fill);
    g.fillRoundedRectangle (area, cornerSize);

    g.setColour (djdeck::layout::colours::Separator);
    g.drawRoundedRectangle (area, cornerSize, 1.0f);
}

//==============================================================================
void RekordboxLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                           bool isMouseOverButton, bool /*isButtonDown*/)
{
    // Botao PLAY do deck: simbolo de play (triangulo) 14 x 17 px.
    if (button.getName() == "deckPlay")
    {
        auto colour = button.findColour (button.getToggleState()
                                        ? juce::TextButton::textColourOnId
                                        : juce::TextButton::textColourOffId);
        if (! button.isEnabled())
            colour = colour.withMultipliedAlpha (0.5f);
        else if (isMouseOverButton)
            colour = djdeck::layout::colours::Primary;

        g.setColour (colour);

        const auto centre = button.getLocalBounds().getCentre().toFloat();
        const float triW = 14.0f;
        const float triH = 17.0f;

        juce::Path playTriangle;
        playTriangle.addTriangle (centre.x - triW * 0.5f, centre.y - triH * 0.5f,
                                  centre.x - triW * 0.5f, centre.y + triH * 0.5f,
                                  centre.x + triW * 0.5f, centre.y);
        g.fillPath (playTriangle);
        return;
    }

    g.setFont (getTextButtonFont (button, button.getHeight()));

    auto colour = button.findColour (button.getToggleState()
                                        ? juce::TextButton::textColourOnId
                                        : juce::TextButton::textColourOffId);

    if (! button.isEnabled())
        colour = colour.withMultipliedAlpha (0.5f);
    else if (isMouseOverButton)
        colour = djdeck::layout::colours::Primary;

    g.setColour (colour);
    g.drawFittedText (button.getButtonText(), 2, 2,
                      button.getWidth() - 4, button.getHeight() - 4,
                      juce::Justification::centred, 2);
}

//==============================================================================
juce::Font RekordboxLookAndFeel::getTextButtonFont (juce::TextButton& button, int buttonHeight)
{
    // Botao CUE do deck: Inter Bold 10 px (medida do Figma: texto 21 x 12 px).
    if (button.getName() == "deckCue")
        return juce::Font (juce::FontOptions ("Inter", 10.0f, juce::Font::bold));

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
