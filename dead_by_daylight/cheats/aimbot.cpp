#include "aimbot.hpp"
#include "actor_manager.hpp"
#include "../math/math.hpp"
#include <windows.h>
#include <algorithm>

#include <fmt/printf.h>
#include <complex>

#include "solver.hpp"

std::optional<cheats::aimbot_t> cheats::aimbot;

void cheats::aimbot_t::run( std::uintptr_t local_player_controller )
{
    const bool wants_default_aim = GetAsyncKeyState( config::options.aim.key );
    const bool wants_predicted_aim = false; //GetAsyncKeyState( config::options.aim.aim_pred_key );

    if ( !wants_default_aim && !wants_predicted_aim ) {
        return;
    }

    auto [ aim_pos, target_vel ] = closest_player_pos();
    if ( aim_pos.is_zero() ) {
        return;
    }

    math::qangle local_angles;
    if ( !m_process.read( local_player_controller + 0x03B0, local_angles ) ) {
        return;
    }

    /*auto normalize = []( math::qangle &a ) {
        while ( a.y() < -180.0f )
            a.y() += 360.0f;
        while ( a.y() > 180.0f )
            a.y() -= 360.0f;

        if ( a.x() >= 319.f ) {
            a.x() = a.x() - 360.f;
        }
    };

    normalize( local_angles );*/

    math::qangle target_angles;
    if ( is_local_player_killer ) {
        auto local_character = get_local_character();
        if ( local_character.empty() ) {
            return;
        }

        float proj_speed = 40.f;
        if ( local_character == "Plague" ) {
            proj_speed = 10.55f;
        } else if ( local_character == "Demogorgon" ) {
            proj_speed = 18.4f;
        } else if ( local_character == "Nurse" ) {
            proj_speed = 13.33f;
        }

        this->m_last_speed_used = proj_speed;

        const float distance = aim_pos.distance_from( actor_manager->local_pos() ) / 100.f /* meters */;
        const float time_to_hit_projectile = distance / proj_speed;

        aim_pos.x() += ( target_vel.x() * time_to_hit_projectile );
        aim_pos.y() += ( target_vel.y() * time_to_hit_projectile );

        target_angles = math::vector_angles( actor_manager->local_pos(), aim_pos );
    } else {
        return;
        // target_angles = math::vector_angles( actor_manager->local_camera_pos(), aim_pos );
    } 
    
    target_angles.y() = 360.f + target_angles.y();
    if ( target_angles.y() > 360.f ) {
        target_angles.y() = target_angles.y() - 360.f;
    }

    target_angles.y() = std::clamp( target_angles.y(), 0.f, 360.f );
    if ( target_angles.x() >= 280.f ) {
        target_angles.x() = target_angles.x() - 360.f;
    }
    if ( target_angles.x() < 0.f ) {
        target_angles.x() = abs( target_angles.x() );
    }

    if ( wants_default_aim ) {
        m_process.write( local_player_controller + 0x3b0 + sizeof( float ), target_angles.y(), sizeof( float ) );
    } else {
        m_process.write( local_player_controller + 0x3b0, target_angles );
    }
}

std::pair<math::vector3, math::vector3> cheats::aimbot_t::closest_player_pos()
{
    auto local_pos = actor_manager->local_pos();

    float best_player_dist = config::options.aim.fov;
    math::vector3 best_player_pos;
    math::vector3 best_player_vel;
    bool playing_as_killer = true;
    bool found_killer = false;

    actor_manager->iterate_actors( [&]( cheats::actor_t *actor ) {
        if ( actor->component().relative_location.is_zero() ) {
            return;
        }

        if ( ( actor->base() == actor_manager->local_actor() ) /*|| !playing_as_killer*/ ) {
            return;
        }

        const auto target_angles = math::vector_angles( local_pos, actor->component().relative_location );
        const auto fov = math::get_fov( actor_manager->local_angles(), target_angles );
        const auto dist = ( actor_manager->local_pos().distance_from( actor->component().relative_location ) / 100.f);

        const auto score = std::max( fov, dist ) / std::min( dist, fov );
        if ( ( actor->tag() == actor_tag_t::killer ) && ( score <= best_player_dist ) ) {
            playing_as_killer = false;
            best_player_pos = actor->component().relative_location;
            best_player_vel = actor->component().velocity;

            return;
        }

        if ( ( actor->tag() == actor_tag_t::survivor ) && ( score <= best_player_dist ) ) {
            const float dist = actor->component().relative_location.distance_from( actor_manager->local_pos() ) / 100.f;

            auto survivor = actor->as<cheats::survivor_t>();
            if ( survivor->in_menu() || !survivor->inner().health_component || ( survivor->health_component().current_health_state_count > 2 ) || ( survivor->health_component().current_health_state_count <= 0 ) ) {
                return;
            }

            if ( dist <= best_player_dist ) {
                best_player_dist = dist;
                best_player_pos = actor->component().relative_location;
                best_player_vel = actor->component().velocity;
            }
        }
    } );

    is_local_player_killer = playing_as_killer;
    return { best_player_pos, best_player_vel };
}

std::string cheats::aimbot_t::get_local_character()
{
    sdk::uobject object;
    m_process.read( actor_manager->local_actor(), object );

    auto raw_name = actor_manager->get_name_for_id( object.name.comparison_index );
    if ( raw_name.empty() ) {
        return "";
    }

    auto actor_info = actor_manager->parse_actor_info( raw_name );
    if ( actor_info.type != actor_tag_t::killer ) {
        return "";
    }

    return actor_info.pretty_name;
}