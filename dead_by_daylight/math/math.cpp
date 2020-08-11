#include "math.hpp"
#include <Windows.h>
#include <cmath>

math::qangle math::vector_angles( const math::vector3 &src )
{
    math::qangle ans;

    if ( src.x() == 0.f && src.y() == 0.f ) {
        ans.x() = ( src.z() > 0.f ) ? 270.f : 90.f;
    } else {
        ans.x() = static_cast<float>( to_degrees( ( atan2( src.z(), src.length_2d() ) ) ) );
        ans.y() = static_cast<float>( to_degrees( atan2( src.y(), src.x() ) ) );
    }

    return ans;
}

math::qangle math::vector_angles( const math::vector3 &a, const math::vector3 &b )
{
    auto delta = a - b;
    auto ans = vector_angles( delta.normalize() );

    if ( ans.y() > 90.f )
        ans.y() -= 180.f;
    else if ( ans.y() < 90.f )
        ans.y() += 180.f;

    return ans;
}

math::vector3 math::angles_vector( const math::qangle &fwd )
{
    math::vector3 ans;
    angle_vectors( fwd, &ans, nullptr, nullptr );

    return ans;
}

void math::angle_vectors( const qangle &angles, vector3 *forward, vector3 *right, vector3 *up )
{
    float angle;
    static float sr, sp, sy, cr, cp, cy;

    angle = to_radians( angles.y() );
    sy = sin( angle );
    cy = cos( angle );
    angle = to_radians( angles.x() );
    sp = sin( angle );
    cp = cos( angle );
    angle = to_radians( angles.z() );
    sr = sin( angle );
    cr = cos( angle );

    if ( forward ) {
        forward->x() = cp * cy;
        forward->y() = cp * sy;
        forward->z() = -sp;
    }

    if ( right ) {
        right->x() = ( -1 * sr * sp * cy + -1 * cr * -sy );
        right->y() = ( -1 * sr * sp * sy + -1 * cr * cy );
        right->z() = -1 * sr * cp;
    }

    if ( up ) {
        up->x() = ( cr * sp * cy + -sr * -sy );
        up->y() = ( cr * sp * sy + -sr * cy );
        up->z() = cr * cp;
    }
}

math::vector2 math::world_to_radar( const math::vector3 &entity_pos, const math::vector3 &local_pos, const math::qangle &local_angles, int width, float scale )
{
    float x_diff = ( entity_pos.x() - local_pos.x() );
    float y_diff = ( entity_pos.y() - local_pos.y() );

    int iRadarRadius = width;

    float flOffset = atanf( y_diff / x_diff );
    flOffset *= 180;
    flOffset /= 3.14159265f;

    if ( ( x_diff < 0 ) && ( y_diff >= 0 ) )
        flOffset = 180 + flOffset;
    else if ( ( x_diff < 0 ) && ( y_diff < 0 ) )
        flOffset = 180 + flOffset;
    else if ( ( x_diff >= 0 ) && ( y_diff < 0 ) )
        flOffset = 360 + flOffset;

    y_diff = -1 * ( sqrtf( ( x_diff * x_diff ) + ( y_diff * y_diff ) ) );
    x_diff = 0;

    flOffset = flOffset - local_angles.y();

    flOffset *= 3.14159265f;
    flOffset /= 180;

    float xnew_diff = x_diff * cosf( flOffset ) - y_diff * sinf( flOffset );
    float ynew_diff = x_diff * sinf( flOffset ) + y_diff * cosf( flOffset );

    xnew_diff /= scale;
    ynew_diff /= scale;

    xnew_diff = ( iRadarRadius / 2 ) + static_cast<int>( xnew_diff );
    ynew_diff = ( iRadarRadius / 2 ) + static_cast<int>( ynew_diff );

    if ( xnew_diff > iRadarRadius )
        xnew_diff = static_cast<float>( iRadarRadius - 4 );
    else if ( xnew_diff < 4 )
        xnew_diff = 4;

    if ( ynew_diff > iRadarRadius )
        ynew_diff = static_cast<float>( iRadarRadius );
    else if ( ynew_diff < 4 )
        ynew_diff = 0;

    return math::vector2( xnew_diff, ynew_diff );
}

void math::vector_transform( const vector3 &in1, const matrix3x4 &in2, vector3 &out )
{
    out.x() = in1.dot( in2[ 0 ] ) + in2[ 0 ][ 3 ];
    out.y() = in1.dot( in2[ 1 ] ) + in2[ 1 ][ 3 ];
    out.z() = in1.dot( in2[ 2 ] ) + in2[ 2 ][ 3 ];
}

math::vector3 math::vector_transform( const vector3 &a, const matrix3x4 &b )
{
    math::vector3 out;
    vector_transform( a, b, out );

    return out;
}

float math::get_fov( const math::qangle &a, const math::qangle &b )
{
    auto delta = ( b - a );
    delta.clamp();

    return delta.length();
}