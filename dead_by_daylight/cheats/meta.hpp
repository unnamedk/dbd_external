#pragma once

#include <unordered_map>
#include <string>
#include <string_view>

namespace cheats::meta
{
    enum class item_rarity : std::uint8_t
    {
        common, // brown
        event, // orange
        uncommon, // yellow
        rare, // green
        very_rare, // purple
        ultra_rare, // magenta

        perk // maybe parse from level? 1 = yellow, 2 = green, 3 = purple
    };

    class item_entry
    {
    public:
        item_entry()
            : m_name( "" )
            , m_description( "" )
            , m_rarity( item_rarity::common )
        {}

        item_entry( std::string_view name, std::string_view description, item_rarity rarity )
            : m_name( name )
            , m_description( description )
            , m_rarity( rarity )
        {}

        item_entry( const item_entry &i )
            : m_name( i.m_name )
            , m_description( i.m_description )
            , m_rarity( i.m_rarity )
        {}

        const std::string &name() const { return this->m_name; }
        const std::string &description() const { return this->m_description; }
        item_rarity rarity() const { return this->m_rarity; }

        std::string &name() { return this->m_name; }
        std::string &description() { return this->m_description; }
        item_rarity &rarity() { return this->m_rarity; }

    private:
        std::string m_name;
        std::string m_description;
        item_rarity m_rarity;
    };

    extern std::unordered_map<std::string_view, std::string> item_translation_table;
    extern std::unordered_map<std::string, item_entry> item_entries;

    extern bool init();
}