#include "CrossfaderComponent.h"
#include "LayoutConfig.hpp"

//==============================================================================
CrossfaderComponent::CrossfaderComponent()
{
    // Posicao: -1.0 (deck A) .. +1.0 (deck B). Centro = mistura equilibrada.
    crossfader.setRange (-1.0, 1.0, 0.001);
    crossfader.setValue (0.0);
    crossfader.setDoubleClickReturnValue (true, 0.0);
    crossfader.addListener (this);
    crossfader.setName ("mixerCrossfader");
    addAndMakeVisible (crossfader);

    // Botoes de atribuicao ao crossfader. O prototipo HTML e o Figma NAO tem
    // esses botoes no mixer (apenas o trilho e o botao do fader), entao
    // ficam ocultos: visiveis, eles cobriam o trilho.
    assignOne.setVisible (false);
    assignTwo.setVisible (false);

    computeGains (0.0, gainA, gainB);
}

CrossfaderComponent::~CrossfaderComponent() = default;

//==============================================================================
void CrossfaderComponent::computeGains (double position, float& gainA, float& gainB)
{
    // Normaliza para 0..1.
    const auto t = juce::jlimit (0.0, 1.0, (position + 1.0) * 0.5);

    // Constant power: as duas somam em potencia constante.
    gainA = static_cast<float> (std::cos (t * juce::MathConstants<double>::halfPi));
    gainB = static_cast<float> (std::sin (t * juce::MathConstants<double>::halfPi));
}

//==============================================================================
void CrossfaderComponent::setPosition (double newPosition)
{
    position = juce::jlimit (-1.0, 1.0, newPosition);
    crossfader.setValue (position, juce::dontSendNotification);

    computeGains (position, gainA, gainB);

    if (onGainsChanged != nullptr)
        onGainsChanged (gainA, gainB);

    repaint();
}

//==============================================================================
void CrossfaderComponent::sliderValueChanged (juce::Slider* slider)
{
    if (slider != &crossfader)
        return;

    position = juce::jlimit (-1.0, 1.0, crossfader.getValue());
    computeGains (position, gainA, gainB);

    if (onGainsChanged != nullptr)
        onGainsChanged (gainA, gainB);

    repaint();
}

//==============================================================================
void CrossfaderComponent::paint (juce::Graphics& g)
{
    // O trilho e o botao sao desenhados pelo LookAndFeel (drawLinearSlider
    // horizontal). Aqui nao ha fundo: a superficie e a do mixer (#0F0F14).
    juce::ignoreUnused (g);
}

//==============================================================================
void CrossfaderComponent::resized()
{
    // O componente dono (CrossfaderComponent) recebe do MixerComponent o
    // envelope do trilho+botao do Figma (99.39 x 10 px). O slider crossfader
    // ocupa o envelope inteiro (0, 0, w, h); o trilho fica centralizado
    // verticalmente dentro dele (centro do trilho = height/2), para que o
    // botao de 10 px nao seja recortado pelo clip do JUCE.
    crossfader.setBounds (0, 0, getWidth(), getHeight());
}
