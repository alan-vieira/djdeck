#include "TagLibFileStream.h"

#include <taglib/toolkit/tiostream.h>
#include <taglib/toolkit/tbytevector.h>

#include <cstdio>

#if defined(_WIN32)
 #include <io.h>
 #include <fcntl.h>
#endif

//==============================================================================
namespace
{
    // Implementacao de TagLib::IOStream sobre um FILE*.
    //
    // O TagLib espera que o streamownership fique com quem o criou: o
    // FileRef NAO deleta o stream.
    class FileStreamImpl : public TagLib::IOStream
    {
    public:
        FileStreamImpl (FILE* handleToUse, const char* nameToUse, bool readOnlyToUse)
            : handle (handleToUse),
              streamName (nameToUse),
              readOnlyFlag (readOnlyToUse)
        {
        }

        ~FileStreamImpl() override
        {
            if (handle != nullptr)
                fclose (handle);
        }

        TagLib::FileName name() const override { return streamName; }

        TagLib::ByteVector readBlock (size_t length) override
        {
            if (handle == nullptr || length == 0)
                return {};

            auto* buffer = new char[length];
            const auto read = fread (buffer, 1, length, handle);

            TagLib::ByteVector result (buffer, static_cast<unsigned int> (read));
            delete[] buffer;

            return result;
        }

        void writeBlock (const TagLib::ByteVector& data) override
        {
            if (handle == nullptr || data.isEmpty())
                return;

            fwrite (data.data(), 1, data.size(), handle);
            fflush (handle);
        }

        // O terceiro parametro e `size_t` (nao `offset_t`) — assinatura da
        // base: `insert(const ByteVector&, offset_t start, size_t replace)`.
        void insert (const TagLib::ByteVector& data,
                     TagLib::offset_t start = 0,
                     size_t replace = 0) override
        {
            if (handle == nullptr || readOnlyFlag || data.isEmpty())
                return;

            const auto fileLength = lengthOfFile();

            if (start < 0 || start > fileLength)
                return;

            // Le a cauda a partir de `start`.
            const auto tailLength = static_cast<size_t> (fileLength - start);
            std::vector<char> tail (tailLength);

            if (tailLength > 0)
            {
                fseek (handle, static_cast<long> (start), SEEK_SET);
                fread (tail.data(), 1, tailLength, handle);
            }

            // Escreve o novo bloco e reposiciona a cauda depois dele.
            fseek (handle, static_cast<long> (start), SEEK_SET);
            fwrite (data.data(), 1, data.size(), handle);

            if (tailLength > 0)
                fwrite (tail.data(), 1, tailLength, handle);

            fflush (handle);
        }

        void removeBlock (TagLib::offset_t start = 0, size_t length = 0) override
        {
            if (handle == nullptr || readOnlyFlag)
                return;

            // Le a cauda do arquivo, apaga o bloco e reescreve a cauda.
            const auto fileLength = lengthOfFile();

            if (start < 0 || start >= fileLength)
                return;

            if (length == 0)
                length = static_cast<size_t> (fileLength - start);

            fseek (handle, 0, SEEK_END);
            const auto tailLength = static_cast<size_t> (fileLength - (start + static_cast<TagLib::offset_t> (length)));

            std::vector<char> tail (tailLength);

            if (tailLength > 0)
            {
                fseek (handle, static_cast<long> (start + static_cast<TagLib::offset_t> (length)), SEEK_SET);
                fread (tail.data(), 1, tailLength, handle);
            }

            truncateTo (start);

            if (tailLength > 0)
            {
                fseek (handle, static_cast<long> (start), SEEK_SET);
                fwrite (tail.data(), 1, tailLength, handle);
                fflush (handle);
            }
        }

        bool readOnly() const override { return readOnlyFlag; }

        bool isOpen() const override { return handle != nullptr; }

        void seek (TagLib::offset_t offset, Position p = Beginning) override
        {
            if (handle == nullptr)
                return;

            int origin = SEEK_SET;

            switch (p)
            {
                case Beginning: origin = SEEK_SET; break;
                case Current:   origin = SEEK_CUR; break;
                case End:       origin = SEEK_END; break;
            }

            fseek (handle, static_cast<long> (offset), origin);
        }

        TagLib::offset_t tell() const override
        {
            if (handle == nullptr)
                return 0;

            return static_cast<TagLib::offset_t> (ftell (handle));
        }

        TagLib::offset_t length() override
        {
            if (handle == nullptr)
                return 0;

            const auto current = ftell (handle);
            fseek (handle, 0, SEEK_END);
            const auto total = ftell (handle);
            fseek (handle, current, SEEK_SET);

            return static_cast<TagLib::offset_t> (total);
        }

        void truncate (TagLib::offset_t newLength) override
        {
            if (handle == nullptr || readOnlyFlag)
                return;

            truncateTo (newLength);
        }

    private:
        TagLib::offset_t lengthOfFile() const
        {
            auto* mutableHandle = const_cast<FILE*> (handle);
            const auto current = ftell (mutableHandle);
            fseek (mutableHandle, 0, SEEK_END);
            const auto total = ftell (mutableHandle);
            fseek (mutableHandle, current, SEEK_SET);

            return static_cast<TagLib::offset_t> (total);
        }

        void truncateTo (TagLib::offset_t newLength)
        {
            if (handle == nullptr)
                return;

            fflush (handle);

         #if defined(_WIN32)
            _chsize_s (_fileno (handle), static_cast<long> (newLength));
         #else
            ftruncate (fileno (handle), static_cast<off_t> (newLength));
         #endif
        }

        FILE* handle;
        const char* streamName;

        // O member NAO pode se chamar `readOnly`: colidiria com o metodo
        // virtual `readOnly()` da classe base, e o compilador deixaria de
        // reconhecer o override (a classe ficaria abstrata).
        bool readOnlyFlag;
    };
}

//==============================================================================
TagLibFileStream::TagLibFileStream (const juce::File& fileToUse, bool readOnlyToUse)
    : file (fileToUse),
      readOnly (readOnlyToUse),
      name (fileToUse.getFullPathName())
{
    #if defined(_WIN32)
        // Caminho LARGO: e a unica forma de abrir caminhos com acento no
        // Windows. `fopen` usaria a codepage ANSI e falharia.
        //
        // O MODO tambem precisa ser largo: `_wfopen` exige `const wchar_t*`
        // nos DOIS argumentos.
        const auto widePath = file.getFullPathName().toUTF16();
        const wchar_t* wideMode = readOnly ? L"rb" : L"rb+";

        handle = _wfopen (widePath.getAddress(), wideMode);
    #else
        const auto mode = readOnly ? "rb" : "rb+";
        handle = fopen (file.getFullPathName().toRawUTF8(), mode);
    #endif

    if (handle != nullptr)
        stream = new FileStreamImpl (static_cast<FILE*> (handle), name.toRawUTF8(), readOnly);
}

TagLibFileStream::~TagLibFileStream()
{
    // O stream e o dono do FILE*: deletar o stream fecha o handle.
    delete stream;
    stream = nullptr;
    handle = nullptr;
}
