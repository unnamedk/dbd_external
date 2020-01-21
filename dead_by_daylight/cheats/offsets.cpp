#include "offsets.hpp"

#include <stdexcept>
#include <fmt/format.h>

std::unique_ptr<cheats::offsets_t> cheats::offsets;

bool cheats::offsets_t::get()
{
    auto from_pattern = [this]( std::string_view pat, std::uint16_t byte_offset ) {
        auto address = m_target_process.find_pattern( pat );
        if ( !address ) {
            throw std::runtime_error( fmt::format( "error looking for pattern \"{}\"", pat ) );
        }

        address += byte_offset;

        std::uint32_t rip = 0u;
        if ( !m_target_process.read( address, rip ) ) {
            throw std::runtime_error( fmt::format( "pattern \"{}\" couldn't be read" ) );
        }

        return ( address + rip + 4 ) - m_target_process.base_address();
    };

    uworld = from_pattern( "48 8B 1D ?? ?? ?? ?? 48 85 DB 74 3B 41", 3 );
    gnames = from_pattern( "48 8B 05 ? ? ? ? 48 85 C0 75 5F", 3 );

    return uworld && gnames;
}