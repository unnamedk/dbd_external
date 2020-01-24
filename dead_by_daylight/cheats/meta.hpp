#pragma once

#include <unordered_map>
#include <string>
#include <string_view>

namespace cheats::meta
{
    class item_entry
    {
    public:
        item_entry()
            : m_name( "" )
            , m_description( "" )
        { }

        item_entry( std::string_view name, std::string_view description )
            : m_name( name )
            , m_description( description )
        {}

        item_entry( const item_entry &i )
            : m_name( i.m_name )
            , m_description( i.m_description )
        {}

        const std::string &name() { return this->m_name; }
        const std::string &description() { return this->m_description; }

    private:
        std::string m_name;
        std::string m_description;
    };

    extern std::unordered_map<std::string_view, std::string> item_translation_table;
    extern std::unordered_map<std::string_view, item_entry> item_entries;
}