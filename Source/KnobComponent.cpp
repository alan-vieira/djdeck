#include "KnobComponent.h"
#include <functional>
#include "LayoutConfig.hpp"

//==============================================================================
KnobComponent::KnobComponent (juce::String labelToUse)
    : caption (std::move (labelToUse)),
      accentColour (djdeck::layout::colours::Accent)
{
    setWantsKeyboardFocus (false);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void KnobComponent::setCaption (const juce::String& text)
{
    caption = text;
    repaint();
}

void KnobComponent::setBipolar (bool shouldBeBipolar)
{
    bipolar = shouldBeBipolar;
    repaint();
}

void KnobComponent::setAccentColour (const juce::Colour& colour)
{
    accentColour = colour;
    repaint();
}

void KnobComponent::setValue (float newValue)
{
    value = juce::jlimit (minValue, maxValue, newValue);
    repaint();
}

void KnobComponent::setRange (float newMin, float newMax)
{
    minValue = newMin;
    maxValue = newMax;
    value = juce::jlimit (minValue, maxValue, value);
    repaint();
}

//==============================================================================
float KnobComponent::getRotaryAngle()
{
    // Mapeia o valor da faixa para 0..1.
    // Para bipolar, o neutro (0) fica em 0.5 — que deve virar o angulo 0.
    const auto proportion = static_cast<float> ((value - minValue) / (maxValue - minValue));

    if (bipolar)
        return (proportion * 2.0f - 1.0f) * (endAngle - startAngle) * 0.5f;

    return startAngle + proportion * (endAngle - startAngle);
}

//==============================================================================
void KnobComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // A faixa do rotulo so e reservada quando ainda sobra espaco para o
    // knob. Nos knobs do mixer (20 x 20 px, medida do Figma) reservar 14 px
    // deixaria um circulo de apenas 6 px de altura — e o `return` abaixo
    // apagaria o knob INTEIRO. Nesse caso o knob ocupa toda a area e o
    // rotulo e omitido (o prototipo HTML tambem nao tem rotulo nos knobs).
    constexpr float captionHeight = 14.0f;
    constexpr float minKnobHeight = 16.0f;

    const bool hasCaptionRoom = caption.isNotEmpty()
                                 && area.getHeight() - captionHeight >= minKnobHeight;

    if (hasCaptionRoom)
        area.removeFromBottom (captionHeight);

    const auto radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 0.5f;

    if (radius <= 2.0f)
        return;

    const auto centre = area.getCentre();
    const auto angle = getRotaryAngle();

    // Corpo
    g.setColour (djdeck::layout::colours::KnobBody);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (djdeck::layout::colours::KnobBorder);
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    // Arco de valor: parte do neutro e cresce ate o valor atual.
    // Num knob bipolar o neutro e o centro da faixa; num unipolar, o minimo.
    {
        const auto centreAngle = bipolar ? 0.0f : startAngle;
        const auto fromAngle = juce::jmin (centreAngle, angle);
        const auto toAngle   = juce::jmax (centreAngle, angle);

        if (toAngle - fromAngle > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (centre.x, centre.y, radius - 1.0f, radius - 1.0f, 0.0f,
                                    fromAngle, toAngle, true);

            g.setColour (accentColour);
            g.strokePath (valueArc, juce::PathStrokeType (2.0f));
        }
    }

    // Indicador
    const auto indicatorLength = radius * 0.7f;
    const auto indicatorX = centre.x + std::sin (angle) * indicatorLength;
    const auto indicatorY = centre.y - std::cos (angle) * indicatorLength;

    g.setColour (djdeck::layout::colours::TextPrimary);
    g.drawLine (centre.x, centre.y, indicatorX, indicatorY, 2.0f);

    // Rotulo (apenas quando a area reservada acima existe)
    if (hasCaptionRoom)
    {
        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.setColour (djdeck::layout::colours::Secondary);
        g.drawText (caption, getLocalBounds().removeFromBottom (static_cast<int> (captionHeight)), juce::Justification::centred, true);
    }
}

//==============================================================================
void KnobComponent::mouseDown (const juce::MouseEvent& e)
{
    lastMouseY = e.position.y;
}

void KnobComponent::mouseDrag (const juce::MouseEvent& e)
{
    const auto deltaY = e.position.y - lastMouseY;
    lastMouseY = e.position.y;

    // Arrastar para cima aumenta, para baixo diminui.
    // Cada 100 pixels = faixa completa.
    const auto change = -static_cast<float> (deltaY) * (maxValue - minValue) / 100.0f;
    setValue (value + change);

    if (onValueChanged != nullptr)
        onValueChanged (value);
}
