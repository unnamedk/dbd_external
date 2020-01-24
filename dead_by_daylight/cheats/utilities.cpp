#include "utilities.hpp"
#include "../config/options.hpp"
#include "../utils/misc.hpp"
#include "../cheats/actor_manager.hpp"
#include <phnt_windows.h>

#include <thread>

std::optional<cheats::utilities_t> cheats::utilities;

#define PALLET_KEY VK_SPACE

void cheats::utilities_t::run_thread()
{
    int time_passed = 0;
    while ( config::options.misc.should_run_threads ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

        // a second has passed
        if ( time_passed++ >= 1'000 ) {
            config::options.misc.should_run_threads = this->m_process.exists();
            time_passed = 0;
        }

        if ( !config::options.misc.master_switch ) {
            continue;
        }

        auto on_local_player = [this]() {
            if ( !actor_manager->local_actor() ) {
                return;
            }

            auto local_player = actor_manager->local_player();

            sdk::aplayercontroller controller;
            if ( !local_player.player_controller || !m_process.read( reinterpret_cast<std::uintptr_t>( local_player.player_controller ), controller ) ) {
                this->m_stillness_time = -1.f;
                return;
            }

            sdk::acamper_player pawn;
            if ( !controller.pawn || !m_process.read( reinterpret_cast<std::uintptr_t>( controller.pawn ), pawn ) ) {
                this->m_stillness_time = -1.f;
                return;
            }

            // cache stillness timer
            sdk::ustillness_tracker tracker;
            if ( m_process.read( reinterpret_cast<std::uintptr_t>( pawn.stillness_tracker ), tracker ) ) {
                this->m_stillness_time = tracker.stillness_timer;
            }

            if ( config::options.misc.autoskillcheck && utils::is_key_pressed( config::options.misc.autoskillcheck_key ) ) {
                this->auto_skillcheck( pawn );
            }
        };
        on_local_player();

        if ( config::options.misc.autopallet && utils::is_key_pressed( config::options.misc.autopallet_key ) ) {
            this->auto_pallet();
        }
    }
}

float cheats::utilities_t::stillness_time()
{
    return this->m_stillness_time;
}

void cheats::utilities_t::auto_skillcheck( sdk::acamper_player &local_pawn )
{
    sdk::uplayer_interaction_handler interaction_handler;
    if ( !local_pawn.interaction_handler || !m_process.read( reinterpret_cast<std::uintptr_t>( local_pawn.interaction_handler ), interaction_handler ) ) {
        return;
    }

    sdk::uplayer_interaction_handler::uskillcheck skillcheck;
    if ( !interaction_handler.skillcheck || !m_process.read( reinterpret_cast<std::uintptr_t>( interaction_handler.skillcheck ), skillcheck ) ) {
        return;
    }

    if ( skillcheck.success_zone_start <= 0.1f ) {
        return;
    }

    if ( ( skillcheck.current_progress_rate >= skillcheck.success_zone_start ) && ( skillcheck.current_progress_rate <= skillcheck.success_zone_end ) ) {
        utils::press_key( VK_SPACE );
    }
}

void cheats::utilities_t::auto_pallet()
{
    math::vector3 nearest_pallet;
    float nearest_pallet_distance = ( std::numeric_limits<float>::max )();

    math::vector3 killer_position;
    math::vector3 killer_velocity;
    bool playing_as_killer = false;

    actor_manager->iterate_actors( [&]( cheats::actor_t *actor ) {
        if ( actor->component().relative_location.is_zero() ) {
            return;
        }

        if ( actor->tag() == actor_tag_t::pallet ) {
            auto pallet = reinterpret_cast<cheats::pallet_t *>( actor );
            if ( pallet->inner().pallet_state != sdk::apallet::pallet_state_t::up ) {
                return;
            }

            auto distance = pallet->component().relative_location.distance_from( actor_manager->local_pos() );
            if ( distance < nearest_pallet_distance ) {
                nearest_pallet = pallet->component().relative_location;
                nearest_pallet_distance = distance;
            }
        }

        if ( actor->tag() == actor_tag_t::killer ) {
            auto killer = reinterpret_cast<cheats::killer_t *>( actor );
            // if killer is unable to attack (invisible, picking up survivor etc) return

            killer_position = killer->component().relative_location;
            killer_velocity = killer->component().velocity;

            playing_as_killer = killer->base() == actor_manager->local_actor();
            if ( playing_as_killer ) {
                return;
            }
        }
    } );

    if ( killer_position.is_zero() || nearest_pallet.is_zero() ) {
        return;
    }

    if ( playing_as_killer ) {
        return; // can't use pallets if it's the killer
    }

    nearest_pallet_distance /= 100.f;
    if ( nearest_pallet_distance > 2.5f ) {
        return; // pallet too far away from player
    }

    killer_position = killer_position + ( killer_velocity * 0.4f );
    if ( ( killer_velocity.length() < 75.f ) && ( ( killer_position.distance_from( actor_manager->local_pos() ) / 100.f ) >= 1.f ) ) {
        return; // killer is way too slow
    }

    if ( ( killer_position.distance_from( nearest_pallet ) / 100.f ) <= 2.3f ) {
        utils::press_key( PALLET_KEY );
    }
}