#pragma once

#include <native/process.hpp>
#include <optional>
#include "../math/elem.hpp"
#include "base_actor.hpp"

#include <string_view>
#include <fmt/format.h>
#include <unordered_map>

namespace cheats
{
    class perk_t
    {
    public:
        perk_t( std::string_view raw_name, int level )
            : m_name( raw_name )
            , m_level( level )
        {}

        const std::string& name() const { return m_name; }
        int level() const { return m_level; }

    private:
        std::string m_name;
        int m_level;
    };

    struct player_info_t
    {
        player_info_t( std::string _name, std::string _icon, std::wstring _platform, std::vector<perk_t> _perks, std::vector<std::string> _addons, std::string _offering, std::string _power, std::uint8_t _roleid, std::int32_t _level )
            : name( _name )
            , icon_name( _icon )
            , platform_id( _platform )
            , perks( _perks )
            , role_id( _roleid )
            , addons( _addons )
            , power( _power )
            , offering( _offering )
            , level( _level )
        {}

        std::string name;
        std::string icon_name;
        std::wstring platform_id;
        std::vector<perk_t> perks;
        std::vector<std::string> addons;
        std::vector<std::string> status_effects;
        std::string offering;
        std::string power;
        std::uint8_t role_id;
        std::int32_t level;
    };

    class esp_t
    {
    public:
        esp_t( const nt::base_process &process )
            : m_process( process )
        {}

        void run();

        //private:
        void draw_name_esp();
        void draw_player_list();
        void draw_radar();

        std::vector<player_info_t> get_players();
        bool should_draw_actor( cheats::actor_t *actor );

        std::vector<std::string> glitched_players;
        const nt::base_process &m_process;
    };

    extern std::optional<esp_t> esp;
}

template <>
struct fmt::formatter<cheats::perk_t>
{
    constexpr auto parse( format_parse_context &ctx ) { return ctx.begin(); }
    template <typename FormatContext>
    auto format( const cheats::perk_t &d, FormatContext &ctx )
    {
        return format_to( ctx.out(), "{}", d.name() );
    }
};