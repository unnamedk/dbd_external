#pragma once

#include <native/process.hpp>
#include <memory>

namespace cheats
{
    class offsets_t
    {
    public:
        offsets_t( const nt::base_process &target_process )
            : m_target_process( target_process )
        { }

        bool get();

        std::uintptr_t gnames = 0ull;
        std::uintptr_t uworld = 0ull;

    private:
        const nt::base_process &m_target_process;
    };

    extern std::unique_ptr<offsets_t> offsets;
}