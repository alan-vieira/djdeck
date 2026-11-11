#include "LibraryStore.h"

#include <juce_core/juce_core.h>

//==============================================================================
juce::File LibraryStore::getLibraryFile()
{
    // No Windows: C:\Users\<usuario>\AppData\Roaming\djdeck\library.xml
    auto folder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);

    if (folder == juce::File())
        folder = juce::File::getSpecialLocation (juce::File::tempDirectory);

    folder = folder.getChildFile ("djdeck");

    if (! folder.exists())
        folder.createDirectory();

    return folder.getChildFile ("library.xml");
}

//==============================================================================
bool LibraryStore::save (const juce::Array<Song>& songs)
{
    const auto file = getLibraryFile();

    juce::XmlElement root ("djdeckLibrary");

    root.setAttribute ("version", 1);

    for (const auto& song : songs)
    {
        auto* element = root.createNewChildElement ("track");

        element->setAttribute ("path", song.file.getFullPathName());
        element->setAttribute ("title", song.title);
        element->setAttribute ("artist", song.artist);
        element->setAttribute ("album", song.album);
        element->setAttribute ("genre", song.genre);
        element->setAttribute ("key", song.key);
        element->setAttribute ("year", song.year);
        element->setAttribute ("trackNumber", song.trackNumber);
        element->setAttribute ("bpm", song.bpm);
        element->setAttribute ("rating", song.rating);
        element->setAttribute ("duration", song.lengthInSeconds);
    }

    // Escreve num arquivo temporario e renomeia: se o app cair no meio da
    // escrita, a biblioteca anterior nao e perdida.
    const auto tempFile = file.getSiblingFile ("library.xml.tmp");

    // writeTo() e a API atual do JUCE 9; writeToFile() esta depreciada.
    if (! root.writeTo (tempFile))
        return false;

    // No Windows o rename falha se o destino existir — apaga o antigo antes.
    file.deleteFile();
    tempFile.moveFileTo (file);

    return true;
}

//==============================================================================
juce::Array<Song> LibraryStore::load()
{
    juce::Array<Song> result;

    const auto file = getLibraryFile();

    if (! file.existsAsFile())
        return result;

    // O parsing de XML pode falhar se o arquivo estiver corrompido (ex: entidades
    // nao suportadas pelo parser JUCE). Envolvemos em try/catch para nao
    // derrubar o app — o usuario pode reimportar as faixas depois.
    try
    {
        juce::XmlDocument document (file);

        // getDocumentElement() retorna um unique_ptr<XmlElement> no JUCE 9.
        auto* root = document.getDocumentElement().get();

        if (root == nullptr || ! root->hasTagName ("djdeckLibrary"))
            return result;

        for (int i = 0; i < root->getNumChildElements(); ++i)
        {
            auto* element = root->getChildElement (i);

            if (element == nullptr || ! element->hasTagName ("track"))
                continue;

            const auto path = element->getStringAttribute ("path");

            if (path.isEmpty())
                continue;

            const juce::File audioFile (path);

            if (! audioFile.existsAsFile())
                continue;

            Song song;
            song.file = audioFile;
            song.title = element->getStringAttribute ("title");
            song.artist = element->getStringAttribute ("artist");
            song.album = element->getStringAttribute ("album");
            song.genre = element->getStringAttribute ("genre");
            song.key = element->getStringAttribute ("key");
            song.year = element->getStringAttribute ("year");
            song.trackNumber = element->getStringAttribute ("trackNumber");
            song.bpm = element->getDoubleAttribute ("bpm", 0.0);
            song.rating = element->getIntAttribute ("rating", 0);
            song.lengthInSeconds = element->getDoubleAttribute ("duration", 0.0);

            result.add (song);
        }
    }
    catch (...)
    {
        // Se o XML estiver corrompido, ignoramos e começamos com biblioteca vazia.
        // O usuario pode reimportar as faixas pelo botao "Adicionar faixa".
    }

    return result;
}
