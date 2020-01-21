#pragma once

#include <native/process.hpp>
#include <optional>
#include "../math/elem.hpp"
#include "base_actor.hpp"

#include <string_view>

namespace cheats
{
    class esp_t
    {
    public:
        esp_t( const nt::base_process &process )
            : m_process( process )
        {}

        void run();

    private:
        void draw_name_esp( );
        void draw_radar( );

        bool should_draw_actor( cheats::actor_t *actor );

        const nt::base_process &m_process;
    };

    extern std::optional<esp_t> esp;
}