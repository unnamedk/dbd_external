#pragma once

#include <native/process.hpp>
#include <optional>
#include "../sdk/game.hpp"

namespace cheats
{
    class utilities_t
    {
    public:
        utilities_t( const nt::base_process &process )
            : m_process( process )
            , m_stillness_time( 0.f )
        {}

        void run_thread();
        float stillness_time();

    private:
        void auto_pallet();
        void auto_skillcheck( sdk::acamper_player& local_pawn );

        const nt::base_process &m_process;
        float m_stillness_time;
    };

    extern std::optional<utilities_t> utilities;
}