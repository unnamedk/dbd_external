#pragma once

#include <native/process.hpp>
#include <optional>
#include "../math/math.hpp"

namespace cheats
{
    class aimbot_t
    {
    public:
        aimbot_t( nt::base_process &process )
            : m_process( process )
        {}
        /// <summary>
        /// 
        /// </summary>
        /// <param name="local_player_controller"></param>
        void run( std::uintptr_t local_player_controller );
        float last_projectile_speed() { return this->m_last_speed_used; }

        bool is_playing_killer() { return this->is_local_player_killer; }

    private:
        void predict( math::vector3 &pos );
        std::pair<math::vector3, math::vector3> closest_player_pos();

        std::string get_local_character();

        float m_last_speed_used = 0.f;
        bool is_local_player_killer = false;

        nt::base_process &m_process;
    };

    extern std::optional<aimbot_t> aimbot;
}