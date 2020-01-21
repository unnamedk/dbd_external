#include "base_actor.hpp"
#include "actor_manager.hpp"

std::string cheats::totem_t::get_hex_name()
{
    if ( this->inner().hex_perk_id.number == 0 ) {
        return "Dull";
    }

    auto name = actor_manager->get_name_for_id( this->inner().hex_perk_id.number );
    auto contains = [&name]( std::string_view str ) {
        return std::search( name.cbegin(), name.cend(), str.cbegin(), str.cend(), []( char A, char B ) {
            return std::tolower( static_cast<int>( A ) ) == std::tolower( static_cast<int>( B ) );
        } ) != name.cend();
    };

    if ( contains( "none" ) ) {
        return "Dull";
    } else if ( contains( "devour" ) ) {
        return "Hex: Devour Hope";
    } else if ( contains( "death" ) ) {
        return "Hex: No One Escapes Death";
    } else if ( contains( "lullaby" ) ) {
        return "Hex: Huntress Lullaby";
    } else if ( contains( "ruin" ) ) {
        return "Hex: Ruin";
    } else if ( contains( "seal" ) ) {
        return "Hex: The Third Seal";
    } else if ( contains( "hunt" ) ) {
        return "Hex: Thrill of the Hunt";
    } else if ( contains( "haunted" ) ) {
        return "Hex: Haunted Ground";
    } else {
        return "Dull";
    }
}

std::string cheats::totem_t::get_icon_name()
{
    if ( this->inner().hex_perk_id.number == 0 ) {
        return "totem";
    }

    auto name = actor_manager->get_name_for_id( this->inner().hex_perk_id.number );
    auto contains = [&name]( std::string_view str ) {
        return std::search( name.cbegin(), name.cend(), str.cbegin(), str.cend(), []( char A, char B ) {
            return std::tolower( static_cast<int>( A ) ) == std::tolower( static_cast<int>( B ) );
        } ) != name.cend();
    };

    if ( contains( "none" ) ) {
        return "totem";
    } else if ( contains( "devour" ) ) {
        return "devourhope";
    } else if ( contains( "death" ) ) {
        return "noed";
    } else if ( contains( "lullaby" ) ) {
        return "huntresslullaby";
    } else if ( contains( "ruin" ) ) {
        return "ruin";
    } else if ( contains( "seal" ) ) {
        return "third_seal";
    } else if ( contains( "hunt" ) ) {
        return "thrill";
    } else if ( contains( "haunted" ) ) {
        return "hauntedground";
    } else {
        return "totem";
    }
}
