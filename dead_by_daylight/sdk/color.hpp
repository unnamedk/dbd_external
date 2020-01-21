#pragma once

#include <array>
#include <nlohmann/json.hpp>

#define impl_MAKECOLOR( name, r, g, b )                  \
    constexpr static color_t name( uint8_t alpha = 255 ) \
    {                                                    \
        return { r, g, b, alpha };                       \
    }

namespace sdk
{
    class color_t
    {
    public:
        constexpr color_t( std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0, std::uint8_t a = 175 )
            : m_buffer { { r, g, b, a } }
        {}

        constexpr color_t( float r, float g = 0.f, float b = 0.f, float a = 0.f )
            : m_buffer {}
        {
            m_buffer[ 0 ] = static_cast<int>( r * 255.f ) & 0xFF;
            m_buffer[ 1 ] = static_cast<int>( g * 255.f ) & 0xFF;
            m_buffer[ 2 ] = static_cast<int>( b * 255.f ) & 0xFF;
            m_buffer[ 3 ] = static_cast<int>( a * 255.f ) & 0xFF;
        }

        constexpr color_t( unsigned int hex )
            : m_buffer {}
        {
            this->m_buffer[ 0 ] = ( ( hex >> 24 ) & 0xFF );
            this->m_buffer[ 1 ] = ( ( hex >> 16 ) & 0xFF );
            this->m_buffer[ 2 ] = ( ( hex >> 8 ) & 0xFF );
            this->m_buffer[ 3 ] = ( ( hex ) &0xFF );
        }

        std::uint8_t *data() { return m_buffer.data(); }
        const std::uint8_t *data() const noexcept { return m_buffer.data(); }

        const uint8_t operator[]( size_t index ) const { return m_buffer[ index ]; }
        uint8_t &operator[]( size_t index ) { return m_buffer[ index ]; }

        int r() const { return m_buffer[ 0 ]; }
        int g() const { return m_buffer[ 1 ]; }
        int b() const { return m_buffer[ 2 ]; }
        int a() const { return m_buffer[ 3 ]; }

        float clamped_r() const { return ( r() != 0 ) ? ( static_cast<float>( r() ) / 255.f ) : 0.f; }
        float clamped_g() const { return ( g() != 0 ) ? ( static_cast<float>( g() ) / 255.f ) : 0.f; }
        float clamped_b() const { return ( b() != 0 ) ? ( static_cast<float>( b() ) / 255.f ) : 0.f; }
        float clamped_a() const { return ( a() != 0 ) ? ( static_cast<float>( a() ) / 255.f ) : 0.f; }

        // predefined colors
        impl_MAKECOLOR( red, 255, 0, 0 );
        impl_MAKECOLOR( blue, 0, 0, 255 );
        impl_MAKECOLOR( green, 0, 255, 0 );

        impl_MAKECOLOR( light_red, 250, 138, 138 );
        impl_MAKECOLOR( light_blue, 173, 216, 230 );
        impl_MAKECOLOR( light_green, 144, 238, 144 );

        impl_MAKECOLOR( white, 255, 255, 255 );
        impl_MAKECOLOR( black, 0, 0, 0 );
        impl_MAKECOLOR( cyan, 0, 255, 255 );
        impl_MAKECOLOR( aquamarine, 127, 255, 212 );
        impl_MAKECOLOR( blue_violet, 138, 43, 226 );
        impl_MAKECOLOR( brown, 165, 42, 42 );
        impl_MAKECOLOR( sky_blue, 135, 206, 235 );
        impl_MAKECOLOR( spring_green, 0, 255, 127 );
        impl_MAKECOLOR( tomato, 255, 99, 71 );
        impl_MAKECOLOR( violet, 238, 130, 238 );
        impl_MAKECOLOR( yellow, 255, 255, 0 );
        impl_MAKECOLOR( turquoise, 64, 224, 208 );
        impl_MAKECOLOR( purple, 128, 0, 128 );
        impl_MAKECOLOR( pink, 255, 192, 203 );
        impl_MAKECOLOR( magenta, 255, 0, 255 );
        impl_MAKECOLOR( indigo, 75, 0, 130 );
        impl_MAKECOLOR( gold, 255, 215, 0 );
        impl_MAKECOLOR( dark_orange, 255, 140, 0 );
        impl_MAKECOLOR( dark_orchid, 153, 50, 204 );
        impl_MAKECOLOR( dark_salmon, 233, 150, 122 );
        impl_MAKECOLOR( orange, 255, 165, 0 );
        impl_MAKECOLOR( orchid, 218, 112, 214 );
        impl_MAKECOLOR( salmon, 250, 128, 114 );
        impl_MAKECOLOR( silver, 192, 192, 192 );

    private:
        std::array<std::uint8_t, 4u> m_buffer;
    };

    void to_json( nlohmann::json &data, const color_t &value );
    void from_json( const nlohmann::json &data, color_t &value );
}