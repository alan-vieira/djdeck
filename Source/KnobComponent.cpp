#include "KnobComponent.h"
#include "RekordboxColours.h"

//==============================================================================
KnobComponent::KnobComponent (juce::String labelToUse)
    : juce::Slider (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      caption (std::move (labelToUse)),
      accentColour (RekordboxColours::accent)
{
    setVelocityBasedMode (false);

    // Arco de 270 graus, com o neutro apontando para cima.
    setRotaryParameters (startAngle, endAngle, false);
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

//==============================================================================
float KnobComponent::getRotaryAngle()
{
    // valueToProportionOfLength mapeia o valor da faixa para 0..1.
    // Para bipolar, o neutro (0) fica em 0.5 — que deve virar o angulo 0.
    const auto proportion = static_cast<float> (valueToProportionOfLength (getValue()));

    if (bipolar)
        return (proportion * 2.0f - 1.0f) * (endAngle - startAngle) * 0.5f;

    return startAngle + proportion * (endAngle - startAngle);
}

//==============================================================================
void KnobComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // A faixa do rotulo e reservada no rodape.
    if (caption.isNotEmpty())
        area.removeFromBottom (14.0f);

    const auto radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 2.0f;

    if (radius <= 2.0f)
        return;

    const auto centre = area.getCentre();
    const auto angle = getRotaryAngle();

    // Corpo
    g.setColour (RekordboxColours::panelRaised);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (RekordboxColours::separator);
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

    g.setColour (RekordboxColours::textPrimary);
    g.drawLine (centre.x, centre.y, indicatorX, indicatorY, 2.0f);

    // Rotulo
    if (caption.isNotEmpty())
    {
        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.setColour (RekordboxColours::textSecondary);
        g.drawText (caption, getLocalBounds().removeFromBottom (14), juce::Justification::centred, true);
    }
}
