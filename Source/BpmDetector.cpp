//==============================================================================
// Detector de BPM por analise de audio.
//
// Implementa o pipeline descrito em BpmDetector.h:
//
//   1. Le os primeiros segundos do arquivo e converte para mono.
//   2. Calcula o envelope de onsets por fluxo espectral (FFT de 2048 pontos,
//      salto de 128 amostras, janela de Hann).
//   3. Autocorrela o envelope nos lags correspondentes a 60..200 BPM e
//      escolhe o pico com maior correlacao. O lag e refinado por
//      interpolacao parabolica para obter resolucao sub-frame.
//   4. Retorna 0.0 quando nao ha sinal suficiente ou quando o pico nao
//      tem correlacao confiavel.
//
// Sem dependencias alem do proprio JUCE.
//==============================================================================

#include "BpmDetector.h"

#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <cstdint>
#include <algorithm>

#include <cmath>
#include <vector>

namespace
{
    //==============================================================================
    // Parametros da analise.

    constexpr int    fftOrder   = 11;                 // 2^11 = 2048 pontos
    constexpr int    fftSize    = 1 << fftOrder;
    constexpr int    hopSize    = 128;                // ~2.9 ms a 44.1 kHz
    constexpr int    numBins    = fftSize / 2 + 1;

    constexpr double minBpm     = 60.0;
    constexpr double maxBpm     = 200.0;

    // Quantos segundos de audio sao analisados. 30 s ja dao amostra estatistica
    // suficiente para a autocorrelacao e mantem o custo do FFT sob controle.
    constexpr double analysisSeconds = 30.0;

    // Correlacao minima exigida do pico. Abaixo disso o resultado e
    // considerado nao confiavel e o detector devolve 0.0.
    constexpr double minConfidence = 0.15;

    // Prior de tempo (log-normal centrada em 120 BPM). Penaliza candidatos
    // muito distantes do centro, o que suprime os picos harmonicos do tipo
    // "metade do tempo" sem eliminar o tempo real.
    constexpr double tempoPriorCentre = 120.0;
    constexpr double tempoPriorSigma  = 0.9;           // em oitavas

    // Fracao do score maximo que um pico local precisa ter para ser aceito
    // como o tempo fundamental. Entre varios picos fortes escolhe o de menor
    // lag (tempo mais rapido), que e o fundamental do envelope de onsets.
    constexpr double peakAcceptance = 0.85;

    //==============================================================================
    // Le o audio do reader e converte para mono (media dos canais).
    //
    // Retorna false se o reader nao tiver samples utilizaveis ou se a leitura
    // falhar.
    bool readMono (juce::AudioFormatReader& reader,
                   std::vector<float>& mono,
                   int64_t numSamples,
                   const std::atomic<bool>* shouldCancel)
    {
        if (numSamples <= 0 || reader.sampleRate <= 0.0)
            return false;

        mono.assign ((size_t) numSamples, 0.0f);

        // O buffer temporario tem sempre 2 canais: quando o arquivo e mono o
        // JUCE duplica o canal esquerdo no segundo slot, entao o downmix abaixo
        // funciona igual para arquivos mono e stereo.
        constexpr int blockSize = 8192;
        juce::AudioBuffer<float> block (2, blockSize);

        const bool sourceIsStereo = reader.numChannels > 1;

        int64_t done = 0;

        while (done < numSamples)
        {
            if (shouldCancel != nullptr && shouldCancel->load())
                return false;

            int chunk = std::min((int) blockSize, (int) (numSamples - done));

            // No JUCE 9 o read() devolve bool: true quando a operacao foi
            // bem-sucedida. Nao existe mais a versao que devolvia o numero de
            // samples lidos.
            if (! reader.read (&block, 0, chunk, done, true, true))
                return false;

            for (int i = 0; i < chunk; ++i)
            {
                const float left  = block.getSample (0, i);
                const float right = block.getSample (1, i);

                mono[(size_t) (done + i)] = sourceIsStereo ? 0.5f * (left + right)
                                                           : left;
            }

            done += chunk;
        }

        return true;
    }

    //==============================================================================
    // Envelope de onsets por fluxo espectral.
    //
    // Para cada frame (janela de Hann de fftSize pontos, avancada de hopSize
    // samples) calcula o espectro de magnitudes e soma o quanto cada bin
    // AUMENTOU em relacao ao frame anterior. Onsets produzem picos positivos.
    bool computeOnsetEnvelope (const std::vector<float>& mono,
                               std::vector<float>& envelope)
    {
        const auto numSamples = (int64_t) mono.size();

        if (numSamples < fftSize)
            return false;

        const auto numFrames = (int) ((numSamples - fftSize) / hopSize + 1);

        if (numFrames < 8)
            return false;

        juce::dsp::FFT fft (fftOrder);
        juce::dsp::WindowingFunction<float> window (fftSize,
                                                    juce::dsp::WindowingFunction<float>::hann);

        // performFrequencyOnlyForwardTransform exige um array com 2 * fftSize
        // floats: a primeira metade contem o dominio do tempo, a segunda e
        // area de trabalho.
        std::vector<float> fftBuffer ((size_t) (2 * fftSize), 0.0f);
        std::vector<float> previousMagnitudes ((size_t) numBins, 0.0f);

        envelope.assign ((size_t) numFrames, 0.0f);

        for (int frame = 0; frame < numFrames; ++frame)
        {
            const auto start = (size_t) frame * hopSize;

            std::copy (mono.begin() + (long) start,
                       mono.begin() + (long) (start + fftSize),
                       fftBuffer.begin());

            window.multiplyWithWindowingTable (fftBuffer.data(), fftSize);

            fft.performFrequencyOnlyForwardTransform (fftBuffer.data(), true);

            // As magnitudes ficam empacotadas em fftBuffer[0 .. numBins - 1].
            float flux = 0.0f;

            for (int bin = 1; bin < numBins; ++bin)
            {
                const float magnitude = fftBuffer[(size_t) bin];
                const float increase  = magnitude - previousMagnitudes[(size_t) bin];

                if (increase > 0.0f)
                    flux += increase;

                previousMagnitudes[(size_t) bin] = magnitude;
            }

            envelope[(size_t) frame] = flux;
        }

        // Remove o nivel medio e retifica: um envelope de onsets deve ter
        // media zero e valores nao negativos.
        float mean = 0.0f;

        for (const auto value : envelope)
            mean += value;

        mean /= (float) envelope.size();

        for (auto& value : envelope)
            value = std::max (0.0f, value - mean);

        return true;
    }

    //==============================================================================
    // Autocorrelacao normalizada do envelope de onsets.
    //
    // Para cada lag valido (lag = frames por batida) devolve o coeficiente de
    // Pearson entre o envelope e ele mesmo deslocado. O resultado fica em
    // [0, 1] para envelopes positivos.
    std::vector<double> autocorrelate (const std::vector<float>& envelope)
    {
        const auto n = (int) envelope.size();
        std::vector<double> correlation;

        if (n < 8)
            return correlation;

        // Energia acumulada, para obter a energia de qualquer janela em O(1).
        std::vector<double> prefixEnergy ((size_t) n + 1, 0.0);

        for (int i = 0; i < n; ++i)
        {
            const double value = envelope[(size_t) i];
            prefixEnergy[(size_t) i + 1] = prefixEnergy[(size_t) i] + value * value;
        }

        const double totalEnergy = prefixEnergy[(size_t) n];

        if (totalEnergy <= 0.0)
            return correlation;   // sinal sem energia: nada a detectar

        const int maxLag = n / 2;

        correlation.resize ((size_t) maxLag + 1, 0.0);

        for (int lag = 1; lag <= maxLag; ++lag)
        {
            double cross = 0.0;

            for (int i = 0; i + lag < n; ++i)
                cross += (double) envelope[(size_t) i] * (double) envelope[(size_t) (i + lag)];

            const double headEnergy = prefixEnergy[(size_t) (n - lag)];
            const double tailEnergy = totalEnergy - prefixEnergy[(size_t) lag];

            const double denominator = std::sqrt (headEnergy * tailEnergy);

            correlation[(size_t) lag] = denominator > 0.0 ? cross / denominator : 0.0;
        }

        return correlation;
    }

    //==============================================================================
    // Peso prior para um BPM candidato (log-normal centrada em 120 BPM).
    double tempoPriorWeight (double bpm)
    {
        if (bpm <= 0.0)
            return 0.0;

        const double octaves = std::log2 (bpm / tempoPriorCentre);

        return std::exp (-0.5 * (octaves / tempoPriorSigma) * (octaves / tempoPriorSigma));
    }
}

//==============================================================================
double BpmDetector::estimate (juce::AudioFormatReader& reader, const std::atomic<bool>* shouldCancel)
{
    if (reader.lengthInSamples <= 0 || reader.sampleRate <= 0.0)
        return 0.0;

    const double sampleRate = reader.sampleRate;

    const auto samplesToRead = std::min((int64_t) reader.lengthInSamples,
                                        (int64_t) (analysisSeconds * sampleRate));

    if (samplesToRead < fftSize)
        return 0.0;

    // 1. Audio mono.
    std::vector<float> mono;

    if (! readMono (reader, mono, samplesToRead, shouldCancel))
        return 0.0;

    if (shouldCancel != nullptr && shouldCancel->load())
        return 0.0;

    // 2. Envelope de onsets.
    std::vector<float> envelope;

    if (! computeOnsetEnvelope (mono, envelope))
        return 0.0;

    // 3. Autocorrelacao.
    const auto correlation = autocorrelate (envelope);

    if (correlation.empty())
        return 0.0;

    // A taxa de frames do envelope determina quantos frames existem por
    // batida para cada BPM candidato.
    const double framesPerSecond = sampleRate / (double) hopSize;

    const int minLag = std::max (2, (int) std::floor (framesPerSecond * 60.0 / maxBpm));
    const int maxLag = std::min ((int) correlation.size() - 1,
                                 (int) std::ceil (framesPerSecond * 60.0 / minBpm));

    if (minLag >= maxLag)
        return 0.0;

    // Score = correlacao ponderada pelo prior de tempo.
    std::vector<double> score ((size_t) (maxLag + 1), 0.0);

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        const double bpm = framesPerSecond * 60.0 / (double) lag;

        score[(size_t) lag] = correlation[(size_t) lag] * tempoPriorWeight (bpm);
    }

    // Coleta os picos locais do score.
    std::vector<int> peaks;

    for (int lag = minLag + 1; lag < maxLag; ++lag)
    {
        if (score[(size_t) lag] > score[(size_t) (lag - 1)]
            && score[(size_t) lag] >= score[(size_t) (lag + 1)])
        {
            peaks.push_back (lag);
        }
    }

    if (peaks.empty())
        return 0.0;

    double bestScore = 0.0;

    for (const auto lag : peaks)
        bestScore = std::max (bestScore, score[(size_t) lag]);

    if (bestScore <= 0.0)
        return 0.0;

    // Entre os picos fortes, escolhe o de MENOR lag: no envelope de onsets o
    // fundamental e o pico mais rapido, e os picos em 2x/3x sao harmonicos.
    int chosenLag = -1;

    for (const auto lag : peaks)
    {
        if (score[(size_t) lag] >= bestScore * peakAcceptance)
        {
            chosenLag = lag;
            break;
        }
    }

    if (chosenLag < 0)
        return 0.0;

    // Confianca: correlacao (nao score) do pico escolhido.
    if (correlation[(size_t) chosenLag] < minConfidence)
        return 0.0;

    // 4. Refino do lag por interpolacao parabolica sobre o score: o pico
    //    raramente cai exatamente num lag inteiro, e sem este refino a
    //    resolucao em BPM seria de ~1 BPM (insuficiente para 128 BPM).
    double refinedLag = (double) chosenLag;

    if (chosenLag > minLag && chosenLag < maxLag)
    {
        const double left  = score[(size_t) (chosenLag - 1)];
        const double mid   = score[(size_t) chosenLag];
        const double right = score[(size_t) (chosenLag + 1)];

        const double denominator = left - 2.0 * mid + right;

        if (std::abs (denominator) > 1.0e-12)
        {
            const double offset = 0.5 * (left - right) / denominator;

            if (offset > -1.0 && offset < 1.0)
                refinedLag += offset;
        }
    }

    if (refinedLag <= 0.0)
        return 0.0;

    const double bpm = framesPerSecond * 60.0 / refinedLag;

    if (bpm < minBpm || bpm > maxBpm)
        return 0.0;

    return bpm;
}

//==============================================================================
void BpmDetector::detectAsync (const juce::File& file,
                               juce::AudioFormatManager& formatManager,
                               std::function<void (double bpm)> callback)
{
    std::shared_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (reader != nullptr) {
        // O callback roda na message thread: e o unico lugar seguro para
        // tocar na UI.
        juce::MessageManager::callAsync ([callback = std::move (callback), reader] () mutable
        {
            double bpm = BpmDetector::estimate (*reader, nullptr);
            if (callback)
                callback (bpm);
        });
    }
}
