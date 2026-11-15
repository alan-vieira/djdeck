#include <catch2/catch_test_macros.hpp>

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <cstdio>

#include "BpmDetector.h"

//==============================================================================
// Gera um WAV de teste com "cliques" periódicos num BPM conhecido, mais um
// pouco de ruido de baixa amplitude. O clique dura 10 ms a cada batida.
namespace
{
    juce::File writeClickWav (double bpm, double seconds, double sampleRate = 44100.0)
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory)
                              .getChildFile ("bpm_test_" + juce::String (bpm) + "bpm.wav");

        // Se ja existir de uma execucao anterior, sobrescreve.
        if (file.existsAsFile())
            file.deleteFile();

        std::unique_ptr<juce::OutputStream> fileStream (file.createOutputStream());
        juce::AudioFormatWriterOptions options;
        options = options.withSampleRate (sampleRate)
                           .withNumChannels (1)
                           .withBitsPerSample (16);
        std::unique_ptr<juce::AudioFormatWriter> writer (
            formatManager.getDefaultFormat()->createWriterFor (fileStream, options));

        if (writer == nullptr)
            return {};

        const int totalSamples = (int) (sampleRate * seconds);
        const int samplesPerBeat = (int) (sampleRate * 60.0 / bpm);
        const int clickSamples = (int) (sampleRate * 0.01); // 10 ms de clique

        juce::AudioBuffer<float> buffer (1, 1024);
        int written = 0;

        while (written < totalSamples)
        {
            const int toWrite = juce::jmin (1024, totalSamples - written);
            buffer.clear ();

            for (int i = 0; i < toWrite; ++i)
            {
                const int pos = written + i;

                if (pos % samplesPerBeat < clickSamples)
                    buffer.setSample (0, i, 0.9f);
            }

            // Ruido deterministico e baixo para nao mascarar os cliques.
            uint32_t seed = (uint32_t) written;
            for (int i = 0; i < toWrite; ++i)
            {
                seed = seed * 1664525u + 1013904223u;
                const float noise = (float) ((seed >> 8) % 1000) / 1000.0f * 0.02f - 0.01f;
                buffer.addSample (0, i, noise);
            }

            writer->writeFromAudioSampleBuffer (buffer, 0, toWrite);
            written += toWrite;
        }

        writer = nullptr; // finaliza o cabeçalho WAV
        return file;
    }
}

//==============================================================================
TEST_CASE ("BpmDetector detecta 128 BPM em sinal de cliques", "[BpmDetector]")
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    const auto file = writeClickWav (128.0, 30.0);
    std::shared_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    std::fprintf (stderr, "[T7] reader=%p\n", (void*) reader.get());
    REQUIRE (reader != nullptr);

    std::fprintf (stderr, "[T8] calling estimate...\n");
    const double        bpm = BpmDetector::estimate (*reader, nullptr);
    std::fprintf (stderr, "[T9] bpm=%f\n", bpm);
    REQUIRE (bpm > 0.0);
    REQUIRE (std::abs (bpm - 128.0) < 2.0);
}

TEST_CASE ("BpmDetector detecta 90 BPM em sinal de cliques", "[BpmDetector]")
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    const auto file = writeClickWav (90.0, 30.0);
    std::shared_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    REQUIRE (reader != nullptr);

    const double        bpm = BpmDetector::estimate (*reader, nullptr);
    REQUIRE (bpm > 0.0);
    REQUIRE (std::abs (bpm - 90.0) < 2.0);
}

TEST_CASE ("BpmDetector detecta 170 BPM em sinal de cliques", "[BpmDetector]")
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    const auto file = writeClickWav (170.0, 30.0);
    std::shared_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    REQUIRE (reader != nullptr);

    const double        bpm = BpmDetector::estimate (*reader, nullptr);
    REQUIRE (bpm > 0.0);
    REQUIRE (std::abs (bpm - 170.0) < 2.0);
}

TEST_CASE ("BpmDetector retorna zero para silencio", "[BpmDetector]")
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    const auto file = juce::File::getSpecialLocation (juce::File::tempDirectory)
                          .getChildFile ("bpm_test_silence.wav");

    if (file.existsAsFile())
        file.deleteFile();

    std::unique_ptr<juce::OutputStream> fileStream (file.createOutputStream());
    juce::AudioFormatWriterOptions options;
    options = options.withSampleRate (44100.0)
                       .withNumChannels (1)
                       .withBitsPerSample (16);
    std::unique_ptr<juce::AudioFormatWriter> writer (
        formatManager.getDefaultFormat()->createWriterFor (fileStream, options));

    juce::AudioBuffer<float> silence (1, 1024);
    silence.clear ();
    int written = 0;
    const int total = (int) (44100.0 * 10.0);

    while (written < total)
    {
        const int toWrite = juce::jmin (1024, total - written);
        writer->writeFromAudioSampleBuffer (silence, 0, toWrite);
        written += toWrite;
    }

    writer = nullptr;

    std::shared_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    REQUIRE (reader != nullptr);

    REQUIRE (BpmDetector::estimate (*reader, nullptr) == 0.0);
}
