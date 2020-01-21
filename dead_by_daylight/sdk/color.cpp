#include "color.hpp"

void sdk::to_json( nlohmann::json &data, const color_t &value )
{
    data = { static_cast<int>( value[ 0 ] ), static_cast<int>( value[ 1 ] ), static_cast<int>( value[ 2 ] ), static_cast<int>( value[ 3 ] ) };
}

void sdk::from_json( const nlohmann::json &data, color_t &value )
{
    auto vec = data.get<std::vector<int>>();

    value[ 0 ] = vec[ 0 ] & 0xFF;
    value[ 1 ] = vec[ 1 ] & 0xFF;
    value[ 2 ] = vec[ 2 ] & 0xFF;
    value[ 3 ] = vec[ 3 ] & 0xFF;
}