#pragma once

#include "base.hpp"

namespace sdk
{
    struct fuobject_item
    {
        uobject *object;
        std::int32_t flags;
        std::int32_t cluster_root_index;
        std::int32_t serial_number;
        std::int32_t unknown;
    };

    struct tuobject_array
    {
        enum
        {
            NumElementsPerChunk = 64 * 1024,
        };

        fuobject_item **Objects;
        fuobject_item *PreAllocatedObjects;
        std::int32_t MaxElements;
        std::int32_t NumElements;
        std::int32_t MaxChunks;
        std::int32_t NumChunks;

        int32_t size() const
        {
            return NumElements;
        }

        int32_t capacity() const
        {
            return MaxElements;
        }

        bool is_valid_index( int32_t Index ) const
        {
            return Index < size() && Index >= 0;
        }

        fuobject_item const *by_id( int32_t Index ) const
        {
            const int32_t ChunkIndex = Index / NumElementsPerChunk;
            const int32_t WithinChunkIndex = Index % NumElementsPerChunk;
            fuobject_item *Chunk = Objects[ ChunkIndex ];
            return Chunk + WithinChunkIndex;
        }
        fuobject_item *by_id( int32_t Index )
        {
            const int32_t ChunkIndex = Index / NumElementsPerChunk;
            const int32_t WithinChunkIndex = Index % NumElementsPerChunk;
            fuobject_item *Chunk = Objects[ ChunkIndex ];
            return Chunk + WithinChunkIndex;
        }

        __forceinline fuobject_item const &operator[]( int32_t Index ) const
        {
            fuobject_item const *ItemPtr = by_id( Index );
            return *ItemPtr;
        }
        __forceinline fuobject_item &operator[]( int32_t Index )
        {
            fuobject_item *ItemPtr = by_id( Index );
            return *ItemPtr;
        }
    };

    struct fuobject_array
    {
        std::int32_t obj_first_gc_index;
        std::int32_t obj_last_non_gc_index;
        std::int32_t max_obj_not_considered_by_gc;
        std::int32_t open_for_disregard_gc;

        tuobject_array obj_objects;
    };

    extern fuobject_array *gobjects;
}