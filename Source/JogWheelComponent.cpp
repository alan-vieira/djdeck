#include "JogWheelComponent.h"
#include "RekordboxColours.h"

//==============================================================================
JogWheelComponent::JogWheelComponent (bool mirroredToUse)
    : mirrored (mirroredToUse)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

JogWheelComponent::~JogWheelComponent() = default;

//==============================================================================
void JogWheelComponent::setBpm (double newBpm)
{
    bpm = newBpm;
    repaint();
}

void JogWheelComponent::setPositionRelative (double pos)
{
    positionRelative = juce::jlimit (0.0, 1.0, pos);
    repaint();
}

void JogWheelComponent::setPitchPercent (double percent)
{
    pitchPercent = static_cast<float> (juce::jlimit (-100.0, 100.0, percent));
    repaint();
}

//==============================================================================
double JogWheelComponent::getEffectiveBpm() const
{
    // O BPM efetivo e o BPM original com a variacao aplicada.
    return bpm * (1.0 + static_cast<double> (pitchPercent) / 100.0);
}

void JogWheelComponent::setMode (Mode newMode)
{
    if (mode == newMode)
        return;

    mode = newMode;

    if (onModeChanged != nullptr)
        onModeChanged (mode);

    repaint();
}

//==============================================================================
float JogWheelComponent::getAngleAt (juce::Point<float> position) const
{
    const auto area = getLocalBounds().toFloat();
    const auto centre = area.getCentre();
    const auto delta = position - centre;

    // 0 = topo, sentido horario. Mesmo sistema do Path::addCentredArc.
    return std::atan2 (delta.x, -delta.y);
}

//==============================================================================
void JogWheelComponent::mouseDown (const juce::MouseEvent& e)
{
    lastAngle = getAngleAt (e.position);
    dragging = true;
}

void JogWheelComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging)
        return;

    const auto angle = getAngleAt (e.position);

    // Diferenca com wrap em ±pi.
    auto delta = angle - lastAngle;

    if (delta > juce::MathConstants<float>::pi)
        delta -= 2.0f * juce::MathConstants<float>::pi;

    if (delta < -juce::MathConstants<float>::pi)
        delta += 2.0f * juce::MathConstants<float>::pi;

    lastAngle = angle;

    // Modo pitch: arrasto vertical.
    if (mode == Mode::pitch)
    {
        const auto change = -e.getDistanceFromDragStartY() * 0.5f;

        pitchPercent = juce::jlimit (-100.0f, 100.0f, pitchPercent + change);
        repaint();
        return;
    }

    // Modo touch: arrasto circular vira deslocamento de posicao.
    // Girar para a direita (delta > 0) AVANCA o audio.
    if (onScratch != nullptr)
        onScratch (delta);
}

void JogWheelComponent::mouseUp (const juce::MouseEvent&)
{
    dragging = false;
}

void JogWheelComponent::mouseDoubleClick (const juce::MouseEvent&)
{
    // Clique no disco alterna o modo.
    setMode (mode == Mode::touch ? Mode::pitch : Mode::touch);
}

//==============================================================================
void JogWheelComponent::drawPitchRange (juce::Graphics& g, const juce::Point<float>& centre,
                                        float radius)
{
    // Marcadores da faixa de pitch na borda do disco.
    // Os segmentos de pitch vao de -8% a +8% na faixa util.
    const auto rangeStart = -juce::MathConstants<float>::pi * 0.25f;
    const auto rangeEnd   =  juce::MathConstants<float>::pi * 0.25f;

    juce::Path pitchSegment;
    pitchSegment.addCentredArc (centre.x, centre.y, radius - 1.0f, radius - 1.0f, 0.0f,
                                rangeStart, rangeEnd, true);

    g.setColour (RekordboxColours::cue);
    g.strokePath (pitchSegment, juce::PathStrokeType (3.0f));
}

//==============================================================================
void JogWheelComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (RekordboxColours::panel);

    const auto radius = static_cast<float> (juce::jmin (area.getWidth(), area.getHeight())) * 0.5f;
    const auto centre = area.getCentre().toFloat();

    // Gira o disco proporcional a posicaoRelativa (1 volta inteira = faixa completa).
    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (
        static_cast<float> (positionRelative * 2.0 * juce::MathConstants<double>::pi),
        centre.x, centre.y));

    // Disco
    g.setColour (RekordboxColours::panelRaised);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (RekordboxColours::separator);
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    // Faixa de pitch na borda.
    drawPitchRange (g, centre, radius);

    g.restoreState();

    // BPM no centro — 28 px, branco (nao gira com o disco).
    g.setFont (juce::Font (juce::FontOptions (28.0f)));
    g.setColour (RekordboxColours::textPrimary);

    const auto bpmText = bpm > 0.0 ? juce::String (getEffectiveBpm(), 2) : juce::String ("--.--");
    g.drawText (bpmText, area.removeFromTop (static_cast<int> (radius)), juce::Justification::centred, true);

    // ---- Variacao de BPM, com setas para cima e para baixo ----
    //
    // A seta indica a DIRECAO do pitch:
    //   ▲  o BPM esta subindo  (variacao positiva)
    //   ▼  o BPM esta descendo (variacao negativa)
    //
    // O valor e mostrado com sinal explicito (+0.00% / -0.00%), porque e a
    // variacao que o DJ le para saber o quanto esta desviando do BPM original.
    const auto isUp = pitchPercent > 0.001f;
    const auto isDown = pitchPercent < -0.001f;

    const auto pitchColour = isUp ? RekordboxColours::cue
                                  : (isDown ? RekordboxColours::loop
                                            : RekordboxColours::textSecondary);

    g.setColour (pitchColour);
    g.setFont (juce::Font (juce::FontOptions (13.0f)));

    const auto percentText = (pitchPercent > 0.001f ? juce::String ("+") : juce::String())
                           + juce::String (pitchPercent, 2) + "%";

    // As setas ficam nas laterais do valor, formando a leitura
    // "▲ +0.00% ▼" que mostra a direcao da variacao.
    auto pitchRow = area.removeFromTop (18);

    const auto arrowSize = 7.0f;
    const auto arrowCentreY = pitchRow.getCentreY();

    // Seta para cima
    if (isUp)
    {
        juce::Path upArrow;
        upArrow.addTriangle (pitchRow.getX() + 2.0f, arrowCentreY - arrowSize * 0.5f,
                             pitchRow.getX() + 2.0f + arrowSize, arrowCentreY - arrowSize * 0.5f,
                             pitchRow.getX() + 2.0f + arrowSize * 0.5f, arrowCentreY + arrowSize * 0.5f);
        g.fillPath (upArrow);
    }

    // Seta para baixo
    if (isDown)
    {
        juce::Path downArrow;
        downArrow.addTriangle (pitchRow.getRight() - 2.0f - arrowSize, arrowCentreY - arrowSize * 0.5f,
                               pitchRow.getRight() - 2.0f, arrowCentreY - arrowSize * 0.5f,
                               pitchRow.getRight() - 2.0f - arrowSize * 0.5f, arrowCentreY + arrowSize * 0.5f);
        g.fillPath (downArrow);
    }

    g.drawText (percentText, pitchRow.reduced (arrowSize + 4, 0), juce::Justification::centred, true);

    // Indicador do modo, no rodape do disco.
    g.setFont (juce::Font (juce::FontOptions (9.0f)));
    g.setColour (RekordboxColours::textTertiary);
    g.drawText (mode == Mode::touch ? "TOUCH" : "PITCH",
                getLocalBounds().removeFromBottom (12), juce::Justification::centred, true);
}
