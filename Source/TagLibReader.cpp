#include "TagLibReader.h"
#include "TagLibFileStream.h"

#include <taglib/fileref.h>
#include <taglib/toolkit/tpropertymap.h>
#include <taglib/toolkit/tstringlist.h>
#include <taglib/audioproperties.h>

//==============================================================================
namespace
{
    // Nomes de tag que o TagLib normaliza entre os formatos.
    // O PropertyMap expoe as chaves em MAIUSCULAS.
    const char* const tagTitle        = "TITLE";
    const char* const tagArtist       = "ARTIST";
    const char* const tagAlbum        = "ALBUM";
    const char* const tagGenre        = "GENRE";
    const char* const tagComment      = "COMMENT";
    const char* const tagBpm          = "BPM";
    const char* const tagTrackNumber  = "TRACKNUMBER";
    const char* const tagDate         = "DATE";
    const char* const tagInitialKey   = "INITIALKEY";
    const char* const tagKey          = "KEY";
    const char* const tagRating       = "RATING";

    // Converte a primeira StringList do PropertyMap numa String do JUCE.
    //
    // to8Bit(true) = UTF-8. O JUCE String precisa de UTF-8 para exibir acentos
    // corretamente.
    juce::String firstValue (const TagLib::StringList& list)
    {
        if (list.isEmpty())
            return {};

        const auto utf8 = list.front().to8Bit (true);
        return juce::String::fromUTF8 (utf8.data(), static_cast<int> (utf8.size()));
    }

    // Converte uma String do JUCE (UTF-8) para TagLib::String.
    //
    // ATENCAO: o construtor `TagLib::String(const char*)` assume LATIN1. Sem
    // o segundo argumento `UTF8`, um titulo com acento viraria lixo.
    TagLib::String toTagLibString (const juce::String& value)
    {
        const auto utf8 = value.toRawUTF8();
        return TagLib::String (utf8, TagLib::String::UTF8);
    }
}

//==============================================================================
TagLibReader::TagLibReader() = default;

TagLibReader::~TagLibReader() = default;

//==============================================================================
TrackMetadata TagLibReader::readMetadata (const juce::File& file) const
{
    TrackMetadata result;

    if (! file.existsAsFile())
        return result;

    const juce::ScopedLock sl (lock);

    // O stream e aberto pelo adaptador, que usa caminho LARGO no Windows.
    TagLibFileStream fileStream (file, true);

    if (! fileStream.isOpen())
        return result;

    TagLib::FileRef ref (fileStream.getStream(),
                          true,
                          TagLib::AudioProperties::Average);

    // No TagLib 2.x a verificacao e `isNull()`, nao `isValid()`.
    if (ref.isNull())
        return result;

    result.isValid = true;

    // ---- Duracao ----
    if (const auto properties = ref.audioProperties())
        result.durationSeconds = properties->lengthInSeconds();

    // ---- Tags ----
    const auto propertyMap = ref.properties();

    result.hasTags = ! propertyMap.isEmpty();

    result.title       = firstValue (propertyMap.value (tagTitle));
    result.artist      = firstValue (propertyMap.value (tagArtist));
    result.album       = firstValue (propertyMap.value (tagAlbum));
    result.genre       = firstValue (propertyMap.value (tagGenre));
    result.comment     = firstValue (propertyMap.value (tagComment));
    result.trackNumber = firstValue (propertyMap.value (tagTrackNumber));
    result.year        = firstValue (propertyMap.value (tagDate));

    // A key pode estar em INITIALKEY (Camelot, como o Rekordbox grava) ou em
    // KEY (notacao classica).
    result.key = firstValue (propertyMap.value (tagInitialKey));

    if (result.key.isEmpty())
        result.key = firstValue (propertyMap.value (tagKey));

    // ---- BPM ----
    const auto bpmText = firstValue (propertyMap.value (tagBpm));

    if (bpmText.isNotEmpty())
    {
        // O BPM pode vir como "128.00" ou "128".
        const auto bpm = bpmText.getDoubleValue();
        result.bpm = (bpm > 0.0 && bpm < 1000.0) ? bpm : 0.0;
    }

    // ---- Rating ----
    // O POPM (ID3) e o RATING (Vorbis) vao de 0 a 5 estrelas.
    const auto ratingText = firstValue (propertyMap.value (tagRating));

    if (ratingText.isNotEmpty())
        result.rating = juce::jlimit (0, 5, static_cast<int> (ratingText.getDoubleValue()));

    // ---- Ano ----
    if (result.year.isNotEmpty())
        result.yearNumber = result.year.getIntValue();

    return result;
}

//==============================================================================
double TagLibReader::readDurationSeconds (const juce::File& file) const
{
    if (! file.existsAsFile())
        return 0.0;

    const juce::ScopedLock sl (lock);

    TagLibFileStream fileStream (file, true);

    if (! fileStream.isOpen())
        return 0.0;

    TagLib::FileRef ref (fileStream.getStream(),
                          true,
                          TagLib::AudioProperties::Average);

    if (ref.isNull())
        return 0.0;

    if (const auto properties = ref.audioProperties())
        return properties->lengthInSeconds();

    return 0.0;
}

//==============================================================================
bool TagLibReader::writeMetadata (const juce::File& file, const TrackMetadata& metadata) const
{
    if (! file.existsAsFile())
        return false;

    const juce::ScopedLock sl (lock);

    // Em modo leitura+escrita: o TagLib grava as tags de volta no mesmo
    // stream.
    TagLibFileStream fileStream (file, false);

    if (! fileStream.isOpen())
        return false;

    TagLib::FileRef ref (fileStream.getStream(),
                          true,
                          TagLib::AudioProperties::Average);

    if (ref.isNull())
        return false;

    TagLib::PropertyMap properties = ref.properties();

    const auto setTag = [&properties] (const char* key, const juce::String& value)
    {
        if (value.isEmpty())
            return;   // campo vazio NAO sobrescreve a tag existente

        properties.replace (key, TagLib::StringList (toTagLibString (value)));
    };

    setTag (tagTitle, metadata.title);
    setTag (tagArtist, metadata.artist);
    setTag (tagAlbum, metadata.album);
    setTag (tagGenre, metadata.genre);
    setTag (tagComment, metadata.comment);
    setTag (tagTrackNumber, metadata.trackNumber);
    setTag (tagKey, metadata.key);

    if (metadata.bpm > 0.0)
        setTag (tagBpm, juce::String (metadata.bpm, 2));

    if (metadata.rating > 0)
        setTag (tagRating, juce::String (metadata.rating));

    ref.setProperties (properties);

    return ref.save();
}
