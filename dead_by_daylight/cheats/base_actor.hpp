#pragma once

#include <cstdint>
#include "../sdk/sdk.hpp"
#include "../config/options.hpp"
#include <native/process.hpp>
#include <string>

namespace cheats
{
    enum class actor_tag_t : std::uint32_t
    {
        unknown,

        // players
        survivor,
        killer,
        item,
        killer_item,

        // objectives
        generator,
        hatch,
        doors,
        totem,

        // environmental
        locker,
        hook,
        pallet,
        chest
    };

    class actor_t
    {
    public:
        actor_t( std::uintptr_t base, actor_tag_t tag, const std::string &pretty_name, const std::string &icon_name, std::string_view raw_name )
            : m_base( base )
            , m_tag( tag )
            , m_name( pretty_name )
            , m_icon_name( icon_name )
            , m_raw_name( raw_name )
        {}

        virtual bool update( const nt::base_process &p ) = 0;
        virtual sdk::uscenecomponent &component() = 0;

        actor_tag_t tag() { return this->m_tag; }

        std::uintptr_t base() { return this->m_base; }
        std::string name() { return this->m_name; }
        std::string_view raw_name() { return this->m_raw_name; }
        std::string icon_name() { return this->m_icon_name; }
        std::int32_t priority() { return config::options.esp.priority_table[ static_cast<std::int32_t>( tag() ) ]; }

        template <typename T>
        T *as()
        {
            return reinterpret_cast<T *>( this );
        }

    protected:
        std::string m_name;
        std::string m_icon_name;
        std::string_view m_raw_name;
        std::uintptr_t m_base;
        actor_tag_t m_tag;
    };

    struct totem_t;
    struct survivor_t;
    template <typename T>
    class base_member : public actor_t
    {
    public:
        using actor_t::actor_t;

        bool update( const nt::base_process &p ) override
        {
            static_assert( std::is_base_of_v<sdk::aactor, T> );

            if ( !p.read( this->base(), member ) ) {
                return false;
            }
            if ( !member.name.number ) {
                return false;
            }

            if ( member.root_component ) {
                if ( !p.read( reinterpret_cast<std::uintptr_t>( member.root_component ), this->m_component ) ) {
                    return false;
                }
            }

            if ( tag() == actor_tag_t::totem ) {
                auto super = reinterpret_cast<totem_t *>( this );
                this->m_icon_name = super->get_icon_name();
                this->m_name = super->get_hex_name();
            }

            else if ( tag() == actor_tag_t::survivor ) {
                auto super = reinterpret_cast<survivor_t *>( this );
                super->update_health_component( p );
            }

            return true;
        }

        auto &inner() { return this->member; }
        sdk::uscenecomponent &component() override { return this->m_component; }

    private:
        T member;
        sdk::uscenecomponent m_component;
    };

    struct default_t : public base_member<sdk::aactor>
    {
        using base_member::base_member;
    };

    struct survivor_t : public base_member<sdk::acamper_player>
    {
        using base_member::base_member;

        void update_health_component( const nt::base_process &p )
        {
            p.read( reinterpret_cast<std::uintptr_t>( inner().health_component ), m_health );
        }
        sdk::ucamper_health_component &health_component() { return m_health; }

    private:
        sdk::ucamper_health_component m_health;
    };

    struct killer_t : public base_member<sdk::aslasher_player>
    {
        using base_member::base_member;
    };

    struct pallet_t : public base_member<sdk::apallet>
    {
        using base_member::base_member;
    };

    struct generator_t : public base_member<sdk::agenerator>
    {
        using base_member::base_member;
    };

    struct totem_t : public base_member<sdk::atotem>
    {
        std::string get_hex_name();
        std::string get_icon_name();

        using base_member::base_member;
    };

    struct hatch_t : public base_member<sdk::ahatch>
    {
        using base_member::base_member;
    };

    struct door_t : public base_member<sdk::aescapedoor>
    {
        using base_member::base_member;
    };

    struct locker_t : public base_member<sdk::alocker>
    {
        using base_member::base_member;
    };

    struct hook_t : public base_member<sdk::ameathook>
    {
        using base_member::base_member;
    };

    struct chest_t : public base_member<sdk::asearchable>
    {
        using base_member::base_member;
    };

    struct item_t : public base_member<sdk::aitem>
    {
        using base_member::base_member;
    };

    struct trap_t : public base_member<sdk::aitem>
    {
        bool is_rbt_remover() { return this->m_raw_name.find( "BP_ReverseBearTrapRemover" ) != std::string_view::npos; }

        void set_rbt_remover( const nt::base_process &p )
        {
            sdk::reverse_bear_trap_remover_t rbt;
            if ( p.read( m_base, rbt ) ) {
                reverse_bear_trap_remover = rbt;
                std::vector<int> keys_;
                keys_.resize( rbt.keys.count );

                p.read_ptr( reinterpret_cast<uintptr_t>( rbt.keys.data ), keys_.data(), keys_.size() * sizeof( int ) );
                keys.emplace( keys_ );
            }
        }

        std::optional<sdk::reverse_bear_trap_remover_t> reverse_bear_trap_remover;
        std::optional < std::vector<int>> keys;

        using base_member::base_member;
    };
}