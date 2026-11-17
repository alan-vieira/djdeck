#include "JogWheelComponent.h"
#include "LayoutConfig.hpp"

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
    juce::Logger::writeToLog ("[DIAG] JogWheelComponent::setBpm called with " + juce::String (newBpm, 2));
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

    g.setColour (djdeck::layout::colours::Cue);
    g.strokePath (pitchSegment, juce::PathStrokeType (3.0f));
}

//==============================================================================
void JogWheelComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.fillAll (djdeck::layout::colours::Panel);

    const auto centre = area.getCentre().toFloat();

    // Gira o disco proporcional a posicaoRelativa (1 volta inteira = faixa completa).
    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (
        static_cast<float> (positionRelative * 2.0 * juce::MathConstants<double>::pi),
        centre.x, centre.y));

    // ---- 3 circulos concêntricos (medidas do Figma) ----
    // Forma: 102 x 102 px (bounds do componente)
    // Meio:  88 x 88 px
    // Dentro: 64 x 64 px
    const float circleRadii[] = { djdeck::layout::jogWheel::diameterPx * 0.5f,
                                  djdeck::layout::jogWheel::innerCirclePx * 0.5f,
                                  djdeck::layout::jogWheel::centerCirclePx * 0.5f };

    // Disco preenchido (circulo externo).
    g.setColour (djdeck::layout::colours::Panelraised);
    g.fillEllipse (centre.x - circleRadii[0], centre.y - circleRadii[0],
                   circleRadii[0] * 2.0f, circleRadii[0] * 2.0f);

    // Bordo externo do disco.
    g.setColour (djdeck::layout::colours::Separator);
    g.drawEllipse (centre.x - circleRadii[0], centre.y - circleRadii[0],
                   circleRadii[0] * 2.0f, circleRadii[0] * 2.0f, 1.0f);

    // Circulos intermediarios (meio e dentro).
    for (int i = 1; i < 3; ++i)
    {
        g.drawEllipse (centre.x - circleRadii[i], centre.y - circleRadii[i],
                       circleRadii[i] * 2.0f, circleRadii[i] * 2.0f, 1.0f);
    }

    // Faixa de pitch na borda (sobre o disco, antes de restaurar o estado).
    drawPitchRange (g, centre, circleRadii[0]);

    g.restoreState();

    // ---- BPM no centro: 58.49 x 20.42 px, Inter Bold 15 (nao gira com o disco) ----
    g.setFont (juce::Font (juce::FontOptions (djdeck::layout::typography::fontFamily,
                                              static_cast<float> (djdeck::layout::jogWheel::bpm::sizePx),
                                              djdeck::layout::jogWheel::bpm::weight)));
    g.setColour (djdeck::layout::colours::Primary);

    const auto bpmText = bpm > 0.0 ? juce::String (getEffectiveBpm(), 2) : juce::String ("--.--");
    const auto bpmArea = juce::Rectangle<float> (
        centre.x - djdeck::layout::jogWheel::bpm::widthPx * 0.5f,
        centre.y - djdeck::layout::jogWheel::bpm::heightPx * 0.5f - 4.0f,
        djdeck::layout::jogWheel::bpm::widthPx, djdeck::layout::jogWheel::bpm::heightPx);
    g.drawText (bpmText, juce::Rectangle<int> ((int) bpmArea.getX(), (int) bpmArea.getY(),
                                               (int) bpmArea.getWidth(), (int) bpmArea.getHeight()),
                juce::Justification::centred, true);

    // ---- Variacao de pitch em relacao ao BPM: 52 x 11 px, Inter Medium 9 ----
    const auto isUp = pitchPercent > 0.001f;
    const auto isDown = pitchPercent < -0.001f;

    const auto pitchColour = isUp ? djdeck::layout::colours::Cue
                                  : (isDown ? djdeck::layout::colours::Loop
                                            : djdeck::layout::colours::Secondary);

    g.setColour (pitchColour);
    g.setFont (juce::Font (juce::FontOptions (djdeck::layout::typography::fontFamily,
                                              static_cast<float> (djdeck::layout::jogWheel::pitch::sizePx),
                                              djdeck::layout::jogWheel::pitch::weight)));

    const auto percentText = (pitchPercent > 0.001f ? juce::String ("+") : juce::String())
                           + juce::String (pitchPercent, 2) + "%";

    const auto pitchArea = juce::Rectangle<float> (
        centre.x - djdeck::layout::jogWheel::pitch::widthPx * 0.5f,
        centre.y + djdeck::layout::jogWheel::bpm::heightPx * 0.5f - 2.0f,
        djdeck::layout::jogWheel::pitch::widthPx, djdeck::layout::jogWheel::pitch::heightPx);
    g.drawText (percentText, juce::Rectangle<int> ((int) pitchArea.getX(), (int) pitchArea.getY(),
                                                   (int) pitchArea.getWidth(), (int) pitchArea.getHeight()),
                juce::Justification::centred, true);

    // Indicador do modo, no rodape do disco.
    g.setFont (juce::Font (juce::FontOptions ("Inter", 9.0f, juce::Font::plain)));
    g.setColour (djdeck::layout::colours::Tertiary);
    g.drawText (mode == Mode::touch ? "TOUCH" : "PITCH",
                getLocalBounds().removeFromBottom (12), juce::Justification::centred, true);
}
