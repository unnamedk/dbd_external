#pragma once

#include <cstdint>

namespace utils
{
    bool is_key_pressed( std::uint16_t code ) noexcept;
    void press_key( std::uint16_t code, std::uint32_t time = 0u ) noexcept;
}