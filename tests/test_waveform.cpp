#include <catch2/catch_test_macros.hpp>

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

//==============================================================================
// Testes do caminho de carga da waveform.
//
// O bug reportado era "os graficos de onda nao aparecem". O caminho e:
//
//   loadURL -> AudioThumbnail::setSource (InputSource*)
//           -> LevelDataSource::createReader
//           -> formatManager.createReaderFor (stream)
//
// Se createReaderFor devolve nullptr, o thumbnail fica sem reader, logo
// getTotalLength() retorna 0 e nada e desenhado. Estes testes medem
// exatamente essa fronteira.
//
// A format manager precisa estar COM O MP3 registrado: em JUCE 9 o
// registerBasicFormats() registra MP3 apenas se JUCE_USE_MP3AUDIOFORMAT == 1.
//
// NOTA SOBRE O HARNESS:
// Um app de console (Catch2) nao cria o MessageManager. O MessageManagerLock
// do JUCE assume MessageManager::instance != nullptr e desreferencia esse
// ponteiro, o que produz SIGSEGV. Por isso getInstance() e chamado antes de
// qualquer lock.
//==============================================================================

namespace
{
    // O app de console precisa que o MessageManager exista antes do primeiro
    // MessageManagerLock.
    struct MessageManagerBootstrap
    {
        MessageManagerBootstrap()
        {
            juce::MessageManager::getInstance();
        }
    };

    MessageManagerBootstrap g_messageManagerBootstrap;

    //==============================================================================
    // Procura uma faixa real SEM usar literal nao-ASCII no codigo.
    //
    // O caminho "D:/Musicas/..." com acento nao pode ser escrito como literal:
    // o compilador pode decodificar o source como Windows-1252 e o caminho
    // resultante deixa de existir. Procurar por NOME resolve o problema de
    // encoding sem depender de codepage.
    juce::File findRealAudioFile()
    {
        if (const char* env = getenv ("DJDECK_TEST_AUDIO"))
        {
            const juce::File fromEnv (env);

            if (fromEnv.existsAsFile())
                return fromEnv;
        }

        const juce::File musicRoot ("D:/");

        if (! musicRoot.isDirectory())
            return {};

        // Procura por profundidade pequena: D:/Musicas/<album>/<faixa>.mp3
        juce::Array<juce::File> candidates;

        for (const auto& dir : musicRoot.findChildFiles (juce::File::findDirectories, false))
        {
            if (dir.isDirectory())
            {
                for (const auto& album : dir.findChildFiles (juce::File::findDirectories, false))
                {
                    if (album.isDirectory())
                    {
                        for (const auto& f : album.findChildFiles (juce::File::findFiles, false))
                        {
                            if (f.getFileExtension().equalsIgnoreCase ("mp3")
                                 && f.getFileName().contains ("Freed From Desire"))
                            {
                                candidates.add (f);
                            }
                        }
                    }
                }
            }
        }

        return candidates.isEmpty() ? juce::File{} : candidates.getReference (0);
    }
}

//==============================================================================
TEST_CASE("O formato MP3 esta registrado na AudioFormatManager", "[Waveform]")
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Sem o MP3 registrado, createReaderFor() devolve nullptr para um .mp3
    // e a waveform nunca aparece.
    REQUIRE (formatManager.findFormatForFileExtension ("mp3") != nullptr);
}

//==============================================================================
TEST_CASE("createReaderFor abre um MP3 real", "[Waveform]")
{
    const auto file = findRealAudioFile();

    if (! file.existsAsFile())
    {
        SKIP ("Nenhum arquivo de audio real disponivel para o teste");
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    auto reader = formatManager.createReaderFor (file);

    REQUIRE (reader != nullptr);
    REQUIRE (reader->lengthInSamples > 0);
    REQUIRE (reader->sampleRate > 0.0);
    REQUIRE (reader->numChannels > 0);
}

//==============================================================================
TEST_CASE("AudioThumbnail com FileInputSource expoe o comprimento total", "[Waveform]")
{
    const auto file = findRealAudioFile();

    if (! file.existsAsFile())
    {
        SKIP ("Nenhum arquivo de audio real disponivel para o teste");
    }

    // setSource() exige o message manager bloqueado.
    const juce::MessageManagerLock messageLock;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    juce::AudioThumbnailCache cache (5);
    juce::AudioThumbnail thumbnail (1024, formatManager, cache);

    thumbnail.setSource (new juce::FileInputSource (file));

    // getTotalLength() e preenchido em setDataSource() a partir do reader.
    // Se for 0, o reader nao foi criado e nada sera desenhado.
    REQUIRE (thumbnail.getTotalLength() > 0.0);
    REQUIRE (thumbnail.getNumChannels() > 0);
}

//==============================================================================
// Replica EXATA do wiring do djdeck:
//
//   MainComponent
//     -> thumbnailCache          (UM cache compartilhado)
//     -> deck1 (MiniWaveformComponent, 1024 amostras por amostra de thumb)
//     -> bigWaveforms (BigWaveformComponent, 2048 amostras por amostra de thumb)
//
// O djdeck usa UM cache para as duas resolucoes. O hash do cache vem do
// ARQUIVO (FileInputSource::hashCode -> file.hashCode), nao da resolucao.
// Mini e big do MESMO arquivo portanto colidem no MESMO slot do cache.
TEST_CASE ("Cache compartilhado: mini e big do mesmo arquivo nao se corrompem", "[Waveform]")
{
    const auto file = findRealAudioFile();

    if (! file.existsAsFile())
        SKIP ("Nenhum arquivo de audio real disponivel para o teste");

    const juce::MessageManagerLock messageLock;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // UM cache, como no app.
    juce::AudioThumbnailCache cache (5);

    juce::AudioThumbnail mini (1024, formatManager, cache);
    juce::AudioThumbnail big (2048, formatManager, cache);

    mini.setSource (new juce::FileInputSource (file));
    big.setSource (new juce::FileInputSource (file));

    REQUIRE (mini.getTotalLength() > 0.0);
    REQUIRE (big.getTotalLength() > 0.0);
}

//==============================================================================
TEST_CASE ("createReaderFor via stream funciona (caminho do thumbnail)", "[Waveform]")
{
    const auto file = findRealAudioFile();

    if (! file.existsAsFile())
        SKIP ("Nenhum arquivo de audio real disponivel para o teste");

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Este e o caminho EXATO que o AudioThumbnail usa:
    // FileInputSource::createInputStream -> createReaderFor (unique_ptr<InputStream>).
    juce::FileInputSource source (file);
    auto stream = source.createInputStream();

    REQUIRE (stream != nullptr);

    auto* reader = formatManager.createReaderFor (std::unique_ptr<juce::InputStream> (stream));

    REQUIRE (reader != nullptr);
    REQUIRE (reader->lengthInSamples > 0);
}

//==============================================================================
TEST_CASE ("Recarregar o mesmo arquivo mantem o comprimento", "[Waveform]")
{
    const auto file = findRealAudioFile();

    if (! file.existsAsFile())
        SKIP ("Nenhum arquivo de audio real disponivel para o teste");

    const juce::MessageManagerLock messageLock;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    juce::AudioThumbnailCache cache (5);
    juce::AudioThumbnail thumbnail (1024, formatManager, cache);

    thumbnail.setSource (new juce::FileInputSource (file));
    const auto firstLength = thumbnail.getTotalLength();

    REQUIRE (firstLength > 0.0);

    // Recarrega o MESMO arquivo: o cache agora tem uma entrada para este hash.
    thumbnail.setSource (nullptr);
    thumbnail.setSource (new juce::FileInputSource (file));

    REQUIRE (thumbnail.getTotalLength() > 0.0);
    REQUIRE (thumbnail.getTotalLength() == firstLength);
}

//==============================================================================
TEST_CASE ("Trocar de arquivo (hash diferente) troca o comprimento", "[Waveform]")
{
    const auto file = findRealAudioFile();

    if (! file.existsAsFile())
        SKIP ("Nenhum arquivo de audio real disponivel para o teste");

    const juce::MessageManagerLock messageLock;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    juce::AudioThumbnailCache cache (5);
    juce::AudioThumbnail thumbnail (1024, formatManager, cache);

    // Arquivo invalido: o thumbnail deve ficar sem reader, sem quebrar.
    thumbnail.setSource (new juce::FileInputSource (juce::File ("D:/arquivo-que-nao-existe.mp3")));

    REQUIRE (thumbnail.getTotalLength() == 0.0);

    // E um arquivo valido deve funcionar depois.
    thumbnail.setSource (new juce::FileInputSource (file));

    REQUIRE (thumbnail.getTotalLength() > 0.0);
}
