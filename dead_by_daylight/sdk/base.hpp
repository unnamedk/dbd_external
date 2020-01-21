#pragma once

#include <cstdint>
#define pad_impl2( x, y ) x##y
#define pad_impl( x, y ) pad_impl2( x, y )
#define pad( x ) pad_impl( std::uint8_t __pad_, __COUNTER__ )[ x ];

namespace sdk
{
    struct fname
    {
        std::int32_t comparison_index;
        std::int32_t number;
    };

    template <typename T>
    struct tarray
    {
        tarray()
            : data( nullptr )
            , count( 0 )
            , max_( 0 )
        {}

        T &operator[]( std::int32_t i ) { return data[ i ]; }
        const T &operator[]( std::int32_t i ) const { return data[ i ]; }

        bool is_valid_index( std::int32_t i ) const { return i < count; }

        T *data;
        std::int32_t count;
        std::int32_t max_;
    };

    template <typename A, typename B>
    struct tpair
    {
        A key;
        B value;
    };

    struct fstring : tarray<wchar_t>
    {};

    struct uclass;
    struct ufunction;
    struct uobject
    {
        void *vtable;
        std::int32_t object_flags;
        std::int32_t internal_index;
        uclass *private_class;
        fname name;
        void *unknown;
        uobject *outer;

        template <typename T>
        T &read( std::uint32_t offset )
        {
            return *reinterpret_cast<T *>( reinterpret_cast<std::uintptr_t>( this ) + offset );
        }
    };

    struct ufield : uobject
    {
        ufield *next;
    };

    struct uenum : ufield
    {
        fstring cpp_type;
        tarray<tpair<fname, std::uint64_t>> names;
        std::int64_t cpp_form;
    };

    struct uproperty;
    struct ustruct : ufield
    {
        pad( 0x10 );
        ustruct *super;
        ufield *children;
        std::int32_t property_size;
        std::int32_t min_alignment;
        tarray<std::uint8_t> script;
        uproperty *property_link;
        uproperty *ref_link;
        uproperty *destructor_link;
        uproperty *post_constructor_link;
        tarray<std::uint8_t> ScriptObjectReferences;
    };

    struct uscriptstruct : ustruct
    {
        pad( 0x10 );
    };

    struct ufunction : ustruct
    {
        std::int32_t flags;
        std::int16_t rep_offset;
        std::int8_t params_num;
        std::int16_t params_size;
        std::int16_t rpc_id;
        std::int16_t rpc_response_id;
        uproperty *first_property_to_init;
        ufunction *event_graph_fn;
        std::int32_t event_graph_call_offset;
        void *function_ptr;
    };

    struct uclass : ustruct
    {
        std::int32_t array_dim;
        std::int32_t elem_size;
        std::uint64_t property_flags;
        std::uint16_t rep_index;
        std::uint8_t blueprint_rep_cond;
        pad( 0x1 );
        std::int32_t offset;
        fname rep_notify_fn;
        uproperty *property_link_next;
        uproperty *next_ref;
        uproperty *destructor_link_next;
        uproperty *post_construct_link_next;
        pad( 0x8 );
    };
}