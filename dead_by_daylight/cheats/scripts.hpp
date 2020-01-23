#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace cheats
{
    class basic_script
    {
    public:
        basic_script( std::string_view name, std::string_view data )
            : m_name( name )
            , m_data( data )
        { }

        const std::string &name() const { return m_name; }
        const std::string &data() const { return m_data; }

        void set_name( std::string_view name ) { m_name = name; }
        void set_data( std::string_view data ) { m_data = data; }

    private:
        std::string m_name;
        std::string m_data;
    };

    void to_json( nlohmann::json &data, const basic_script &value );
    void from_json( const nlohmann::json &data, basic_script &value );
}