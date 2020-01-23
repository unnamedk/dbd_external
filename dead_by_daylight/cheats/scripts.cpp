#include "scripts.hpp"

void cheats::to_json( nlohmann::json &data, const basic_script &value )
{
    data = {
        std::make_pair( value.name(), value.data() )
    };
}

void cheats::from_json( const nlohmann::json &data, basic_script &value )
{
    std::pair<std::string, std::string> read = data.get<std::pair<std::string, std::string>>();
    value.set_name( read.first );
    value.set_data( read.second );
}
