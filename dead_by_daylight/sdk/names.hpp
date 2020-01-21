#pragma once

#include <cstdint>

namespace sdk
{
    struct fname_entry
    {
        fname_entry *hash_next;
        std::int32_t index;

        union
        {
            char ansi_name[ 256 ];
            wchar_t wide_name[ 256 ];
        };

        char* get_name()
        {
            return ansi_name;
        }
    };

    template <typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
    class TStaticIndirectArrayThreadSafeRead
    {
    public:
        enum
        {
            // figure out how many elements we need in the master table
            ChunkTableSize = ( MaxTotalElements + ElementsPerChunk - 1 ) / ElementsPerChunk
        };

        int32_t size() const
        {
            return NumElements;
        }

        bool is_valid_index( int32_t index ) const
        {
            return index < size() && index >= 0 && by_id( index ) != nullptr;
        }

        ElementType const *const &by_id( int32_t index ) const
        {
            return *get_item_ptr( index );
        }

    private:
        ElementType const *const *get_item_ptr( int32_t Index ) const
        {
            int32_t ChunkIndex = Index / ElementsPerChunk;
            int32_t WithinChunkIndex = Index % ElementsPerChunk;
            ElementType **Chunk = Chunks[ ChunkIndex ];
            return Chunk + WithinChunkIndex;
        }

        ElementType **Chunks[ ChunkTableSize ];
        __int32 NumElements;
        __int32 NumChunks;
    };

    using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<fname_entry, 4 * 1024 * 1024, 16384>;
    extern TNameEntryArray *gnames;
}