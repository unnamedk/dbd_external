#pragma once

#include "elem.hpp"

namespace math
{
    constexpr auto pi = 3.14159265358979323846f;
    constexpr auto rad_pi = 180.f / pi;
    constexpr auto pi_rad = pi / 180.f;
    constexpr auto golden_ratio = 1.6180339887f;

    constexpr auto to_degrees( float rad ) { return rad * rad_pi; }
    constexpr auto to_radians( float deg ) { return deg * pi_rad; }
    math::qangle vector_angles( const math::vector3 &src );
    math::qangle vector_angles( const math::vector3 &a, const math::vector3 &b );
    math::vector3 angles_vector( const math::qangle &fwd );
    void angle_vectors( const qangle &angles, vector3 *forward, vector3 *right, vector3 *up );

    math::vector2 world_to_radar( const math::vector3 &entity_pos, const math::vector3 &local_pos, const math::qangle &local_angles, int width, float scale = 16.f );

    void vector_transform( const vector3 &in1, const matrix3x4 &in2, vector3 &out );
    vector3 vector_transform( const vector3 &a, const matrix3x4 &b );

    float get_fov( const math::qangle &a, const math::qangle &b );

	template <std::size_t N>
    constexpr __forceinline std::uint64_t fnv_constexpr( const char ( &str )[ N ] )
    {
        constexpr auto prime = 0x100000001b3;

        auto ans = 0xcbf29ce484222325;
        for ( int i = 0; i < ( N - 1 ); ++i ) {
            ans = ( ans ^ str[ i ] ) * prime;
        }

        return ans;
    }

    constexpr __forceinline std::uint64_t fnv_runtime( const char *str, int len )
    {
        constexpr auto prime = 0x100000001b3;

        auto ans = 0xcbf29ce484222325;
        for ( int i = 0; i < len; ++i ) {
            ans = ( ans ^ str[ i ] ) * prime;
        }

        return ans;
    }
}