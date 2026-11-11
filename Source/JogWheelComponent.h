#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
// Jog wheel — o disco de manipulacao.
//
// NAO e um juce::Slider: o Slider nao tem o conceito de rotacao livre com
// inercia. E um Component customizado que captura os eventos de mouse.
//
// Dois modos, alternados pelo clique no disco:
//
//   | Modo    | Arrasto     | Efeito                              |
//   |---------|-------------|-------------------------------------|
//   | Touch   | circular    | faz "scratch" — move a posicao      |
//   | Pitch   | vertical    | ajusta a velocidade de reproducao  |
//
// No modo touch, o angulo de arrasto do mouse mapeia para deslocamento de
// posicao. Girar o disco PARA A DIREITA avanca o audio.
//
// O `mirrored` so move os botoes laterais (CUE, play/pause) — nao inverte o
// sentido da rotacao.
//
// Ver docs/DOCUMENTACAO_LAYOUT_UI.md §6.3.
class JogWheelComponent : public juce::Component
{
public:
    explicit JogWheelComponent (bool mirroredToUse);
    ~JogWheelComponent() override;

    void paint (juce::Graphics& g) override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    // BPM exibido no centro do disco.
    void setBpm (double bpm);
    double getBpm() const { return bpm; }

    // Posicao relativa de reproducao (0.0 a 1.0). Gira o disco proporcionalmente
    // para acompanhar a faixa que esta tocando.
    void setPositionRelative (double pos);
    double getPositionRelative() const { return positionRelative; }

    // Variacao de BPM, em percent (-100 .. +100). O valor real do BPM
    // resultante e BPM * (1 + percent/100).
    void setPitchPercent (double percent);
    double getPitchPercent() const { return pitchPercent; }

    // BPM resultante, ja com a variacao aplicada.
    double getEffectiveBpm() const;

    // Modo atual do disco.
    enum class Mode { touch, pitch };
    Mode getMode() const { return mode; }
    void setMode (Mode newMode);

    // Chamado no arrasto circular: `deltaRadians` e o deslocamento angular
    // acumulado (ja com wrap em ±pi). Girar para a direita AVANCA o audio.
    std::function<void (float deltaRadians)> onScratch;

    // Chamado quando o modo muda por clique no disco.
    std::function<void (Mode)> onModeChanged;

private:
    // Angulo do ponteiro em torno do centro, em radianos, 0 = topo.
    float getAngleAt (juce::Point<float> position) const;

    void drawPitchRange (juce::Graphics& g, const juce::Point<float>& centre, float radius);

    bool mirrored;
    bool dragging = false;

    float lastAngle = 0.0f;
    float pitchPercent = 0.0;
    double bpm = 0.0;
    double positionRelative = 0.0;

    Mode mode = Mode::touch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JogWheelComponent)
};
