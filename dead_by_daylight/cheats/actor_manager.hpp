#pragma once

#include <native/process.hpp>
#include <unordered_map>

#include "../sdk/sdk.hpp"
#include <mutex>
#include <optional>
#include <memory>
#include "base_actor.hpp"

namespace cheats
{
    struct actor_info
    {
        std::string pretty_name;
        actor_tag_t type;

        std::string name_to_icon()
        {
            std::string name = pretty_name;
            std::transform( name.begin(), name.end(), name.begin(), []( char c ) {
                if ( c == ' ' ) {
                    return '_';
                }

                return static_cast<char>( ::tolower( static_cast<int>( c ) ) );
            } );

            return name;
        }
    };

    class actor_manager_t
    {
    public:
        actor_manager_t( const nt::base_process &process )
            : m_process( process )
            , m_local_actor( 0ull )
        {}

        void run_thread();

        std::string_view get_name_for_id( std::uint32_t id );
        void update_names();

        template <typename Pred>
        void iterate_actors( Pred &&pred )
        {
            std::unique_lock lock { m_array_lock };
            for ( auto &a : m_actors ) {
                if ( a /*&& ( ( a->component().relative_location.distance_from( m_local_pos ) / 100.f ) <= 200.f )*/ ) {
                    pred( a.get() );
                }
            }
        }

        math::vector3 local_pos() { return this->m_local_pos; }
        math::vector3 local_camera_pos() { return this->m_local_camera_pos; }
        math::qangle local_angles() { return this->m_local_angles; }
        math::vector3 local_velocity() { return this->m_local_velocity; }
        std::uintptr_t local_actor() { return this->m_local_actor; }
        sdk::ulocalplayer local_player() { return this->m_local_player; }

        std::optional<sdk::aplayercontroller> local_controller()
        {
            sdk::aplayercontroller ans;
            if ( !this->m_process.read( uintptr_t( m_local_player.player_controller ), ans ) ) {
                return std::nullopt;
            }

            return ans;
        }

        auto get_game_state() { return game_state; }
        void dump_names() noexcept;

        bool is_in_terror_radius() { return this->m_in_terror_radius; }
        actor_info parse_actor_info( std::string_view name );

    private:

        const nt::base_process &m_process;

        bool m_in_terror_radius = false;

        sdk::ugameinstance instance;
        sdk::ulocalplayer m_local_player;

        std::mutex m_array_lock;
        std::vector<std::unique_ptr<cheats::actor_t>> m_actors;

        std::mutex m_name_lock;
        std::unordered_map<std::uint32_t, std::string> m_names;

        sdk::udbd_game_state game_state;

        math::vector3 m_local_pos;
        math::vector3 m_local_camera_pos;
        math::qangle m_local_angles;
        math::vector3 m_local_velocity;
        std::uintptr_t m_local_actor;
    };

    extern std::optional<actor_manager_t> actor_manager;
}