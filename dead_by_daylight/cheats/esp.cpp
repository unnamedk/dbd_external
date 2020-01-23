#include "esp.hpp"
#include "actor_manager.hpp"
#include "../overlay/imgui/imgui.h"

#include "../config/options.hpp"
#include <algorithm>
#include <DirectXTex.h>
#include <magic_enum.hpp>

#include "../overlay/menu.hpp"
#include "../math/math.hpp"
#include "../utils/misc.hpp"
#include "utilities.hpp"
#include <native/utils.hpp>

#include <fmt/printf.h>
#include <phnt_windows.h>

std::optional<cheats::esp_t> cheats::esp;
std::unordered_map<std::string_view, std::string> cheats::perk_name_table = {
    // powers/items
    { "Item_Slasher_Frenzy", "Frenzy" },

    // addons
    { "Addon_Spark_IridescentGeneral", "Iridescent King" },
    { "Addon_Spark_JadeCharm", "Iridescent Queen" },

    // killer perks
    { "ZanshinTactics", "Zanshin Tactics" },
    { "Unnerving_Presence", "Unnerving Presence" },
    { "SpiritFury", "Spirit Fury" },
    { "Sloppy_Butcher", "Sloppy Butcher" },
    { "OverwhelmingPresence", "Overwhelming Presence" },
    { "InTheDark", "Knock Out" },
    { "Iron_Grasp", "Iron Grasp" },
    { "FireUp", "Fire Up" },
    { "HangmansTrick", "Hangman's Trick" },
    { "Hex_Thrill_Of_The_Hunt", "Hex: Thrill of the Hunt" },
    { "ImAllEars", " I'm All Ears" },
    { "CorruptIntervention", "Corrupt Intervention" },
    { "BeastOfPrey", "Beast of Prey" },
    { "ThrillingTremors", "Thrilling Tremors" },
    { "TerritorialImperative", "Territorial Imperative" },
    { "Spies_From_The_Shadows", "Spies from the shadows" },
    { "Save_The_Best_For_Last", "Save the Best for Last" },
    { "RememberMe", "Remember Me" },
    { "pop_goes_the_weasel", "Pop Goes The Weasel" },
    { "Play_With_Your_Food", "Play With Your Food" },
    { "GeneratorOvercharge", "Overcharge" },
    { "Monstrous_Shrine", "Monstrous Shrine" },
    { "MadGrit", "Mad Grit" },
    { "MakeYourChoice", "Make Your Choice" },
    { "MonitorAndAbuse", "Monitor and Abuse" },
    { "IronMaiden", "Iron Maiden" },
    { "InfectiousFright", "Infectious Fright" },
    { "Hex_The_Third_Seal", "Hex: The Third Seal" },
    { "Hex_HauntedGround", "Hex: Haunted Ground" },
    { "Hex_HuntressLullaby", "Hex: Huntress Lullaby" },
    { "No_One_Escapes_Death", "Hex: No One Escapes Death" },
    { "Hex_Ruin", "Hex: Ruin" },
    { "Hex_Devour_Hope", "Hex: Devour Hope" },
    { "FurtiveChase", "Furtive Chase" },
    { "FranklinsLoss", "Franklin's Demise" },
    { "Dying_Light", "Dying Light" },
    { "DarkDevotion", "Dark Devotion" },
    { "CruelConfinement", "Cruel Limits" },
    { "Brutal_Strength", "Brutal Strength" },
    { "BloodWarden", "Bloodwarden" },
    { "BloodEcho", "Blood Echo" },
    { "Bitter_Murmur", "Bitter Murmur" },
    { "NurseCalling", "Nurse's Calling" },
    { "BBQAndChili", "Barbecue and Chili" },
};

inline void
    set_tooltip( const char *str )
{
    ImGui::SameLine();
    //ImGui::TextDisabled( "(?)" );
    if ( ImGui::IsItemHovered() ) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos( 450.0f );
        ImGui::TextUnformatted( str );
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void cheats::esp_t::run()
{
    this->draw_name_esp();
    this->draw_radar();
}

void cheats::esp_t::draw_name_esp()
{
    ImGui::Separator();
    if ( cheats::utilities->stillness_time() != -1.f ) {
        ImGui::Text( "stillness timer: %.2f", cheats::utilities->stillness_time() );
    }
    ImGui::BeginChild( "name_esp", {}, true );

    ImGui::Columns( config::options.esp.debug_mode ? 3 : 2 );
    ImGui::Separator();
    ImGui::Text( "name" );
    ImGui::NextColumn();
    ImGui::Text( "distance" );
    ImGui::NextColumn();
    if ( config::options.esp.debug_mode ) {
        ImGui::Text( "pos" );
        ImGui::NextColumn();
    }

    std::vector<std::tuple<std::string, int, math::vector3>> names;
    names.reserve( 30 );

    actor_manager->iterate_actors( [this, &names]( cheats::actor_t *actor ) {
        if ( !actor || !should_draw_actor( actor ) ) {
            return;
        }

        math::vector3 pos;
        switch ( actor->tag() ) {
            case actor_tag_t::generator: {
                auto gen = ( ( cheats::generator_t * ) actor );
                if ( gen->inner().is_repaired ) {
                    return;
                }

                std::string text;
                text.reserve( 25 );

                if ( gen->inner().is_blocked ) {
                    text = "Generator (blocked)";
                } else {
                    text = fmt::sprintf( "Generator (%.2f %%)", gen->inner().native_percent_complete * 100.f );
                }

                names.emplace_back( text, gen->priority(), gen->component().relative_location );
                break;
            }

            case actor_tag_t::totem: {
                auto hex_name = reinterpret_cast<cheats::totem_t *>( actor )->get_hex_name();
                names.emplace_back( fmt::sprintf( "Totem (%s)", hex_name.data() ), actor->priority(), actor->component().relative_location );
                break;
            }

            case actor_tag_t::hatch: {
                auto hatchstate = ( reinterpret_cast<cheats::hatch_t *>( actor )->inner().hatch_state );
                if ( ( hatchstate == sdk::ahatch::hatch_state_t::default_open ) || ( hatchstate == sdk::ahatch::hatch_state_t::key_open ) ) {
                    names.emplace_back( "Hatch (open)", actor->priority(), actor->component().relative_location );
                } else {
                    names.emplace_back( "Hatch (closed)", actor->priority(), actor->component().relative_location );
                }

                break;
            }

            /*case actor_tag_t::killer: {
                auto killer = reinterpret_cast<cheats::killer_t *>( actor );
                if ( killer->inner().is_killing ) {
                    names.emplace_back( fmt::format( "{} (in mori)", killer->name() ), killer->priority(), killer->component().relative_location );
                } else {
                    names.emplace_back( fmt::format( "{} ( {} {} )", killer->inner().allowed_kill_count, killer->inner().allowed_kill_last_survivor ), killer->priority(), killer->component().relative_location );
                }
            }*/
            case actor_tag_t::survivor: {
                auto survivor = reinterpret_cast<cheats::survivor_t *>( actor );
                if ( ( survivor->health_component().current_health_state_count > 2 ) || ( survivor->health_component().current_health_state_count < -1 ) ) {
                    names.emplace_back( fmt::sprintf( "%s", survivor->name().data() ), actor->priority(), actor->component().relative_location );
                } else if ( survivor->health_component().current_health_state_count == -1 ) {
                    names.emplace_back( fmt::sprintf( "%s (dead)", survivor->name().data() ), actor->priority(), actor->component().relative_location );
                } else {
                    names.emplace_back( fmt::sprintf( "%s (%i HP)", survivor->name().data(), survivor->health_component().current_health_state_count ), actor->priority(), actor->component().relative_location );
                }
                break;
            }
            default:
                names.emplace_back( actor->name(), actor->priority(), actor->component().relative_location );
        }
    } );

    std::sort( names.begin(), names.end(), []( auto &a, auto &b ) {
        return ( std::get<1>( a ) > std::get<1>( b ) );
    } );

    for ( auto &[ name, _, pos ] : names ) {
        if ( pos.is_zero() ) {
            continue;
        }

        ImGui::Text( "%s\n", name.data() );

        ImGui::NextColumn();
        {
            ImGui::Text( "%.1f M", pos.distance_from( actor_manager->local_pos() ) / 100.f );
        }

        ImGui::NextColumn();
        if ( config::options.esp.debug_mode ) {
            ImGui::Text( "[ %.2f %.2f %.2f ]", pos.x(), pos.y(), pos.z() );
            ImGui::NextColumn();
        }
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void cheats::esp_t::draw_player_list()
{
    ImGui::Columns( 7 );
    ImGui::Text( "character" );
    ImGui::NextColumn();

    ImGui::Text( "platform id" );
    ImGui::NextColumn();

    ImGui::Text( "perks" );
    ImGui::NextColumn();

    ImGui::Text( "addons" );
    ImGui::NextColumn();

    ImGui::Text( "offering" );
    ImGui::NextColumn();

    ImGui::Text( "power/item" );
    ImGui::NextColumn();

    ImGui::Text( "role id" );
    ImGui::NextColumn();

    auto players = get_players();
    int i = 0;
    for ( auto &p : players ) {
        ImGui::Text( "%s", p.name.data() );
        ImGui::NextColumn();

        auto plat_id = nt::utils::narrow( p.platform_id );

        ImGui::Text( "%s", plat_id.empty() ? "unavailable" : plat_id.data() );
        ImGui::SameLine();
        ImGui::PushID( i++ );
        if ( ImGui::Button( "copy url" ) ) {
            auto url = fmt::format( "https://steamcommunity.com/profiles/{}", plat_id );
            if ( OpenClipboard( nullptr ) ) {
                auto mem = GlobalAlloc( GMEM_MOVEABLE, url.size() + 1 );
                memcpy( GlobalLock( mem ), url.data(), url.size() + 1 );

                GlobalUnlock( mem );
                EmptyClipboard();
                SetClipboardData( CF_TEXT, mem );
                CloseClipboard();
            }
        }
        ImGui::PopID();
        ImGui::NextColumn();

        auto perks = fmt::format( "{}", fmt::join( p.perks, ", " ) );

        ImGui::Text( "%s", perks.data() );
        ImGui::NextColumn();

        auto addons = fmt::format( "{}", fmt::join( p.addons, ", " ) );
        ImGui::Text( "%s", addons.data() );
        ImGui::NextColumn();

        ImGui::Text( "%s", p.offering.data() );
        ImGui::NextColumn();

        ImGui::Text( "%s", p.power.data() );
        ImGui::NextColumn();

        std::string_view role_name = ( p.role_id == 1 ) ? "killer" : "survivor";

        ImGui::Text( "%s", role_name.data() );
        ImGui::NextColumn();
    }
}

void cheats::esp_t::draw_radar()
{
    ImGui::SetNextWindowSizeConstraints( ImVec2( 0, 0 ), ImVec2( FLT_MAX, FLT_MAX ), []( ImGuiSizeCallbackData *data ) {
        data->DesiredSize = ImVec2( std::max( data->DesiredSize.x, data->DesiredSize.y ), std::max( data->DesiredSize.x, data->DesiredSize.y ) );
    } );
    if ( !ImGui::Begin( "radar", &config::options.esp.radar, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar ) ) {
        return;
    }

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 winpos = ImGui::GetWindowPos();
    ImVec2 winsize = ImGui::GetWindowSize();

    draw_list->AddLine( ImVec2( winpos.x + winsize.x * 0.5f, winpos.y ), ImVec2( winpos.x + winsize.x * 0.5f, winpos.y + winsize.y ), ImColor( 70, 70, 70, 255 ), 1.f );
    draw_list->AddLine( ImVec2( winpos.x, winpos.y + winsize.y * 0.5f ), ImVec2( winpos.x + winsize.x, winpos.y + winsize.y * 0.5f ), ImColor( 70, 70, 70, 255 ), 1.f );

    draw_list->AddLine( ImVec2( winpos.x + winsize.x * 0.5f, winpos.y + winsize.y * 0.5f ), ImVec2( winpos.x, winpos.y ), ImColor( 90, 90, 90, 255 ), 1.f );
    draw_list->AddLine( ImVec2( winpos.x + winsize.x * 0.5f, winpos.y + winsize.y * 0.5f ), ImVec2( winpos.x + winsize.x, winpos.y ), ImColor( 90, 90, 90, 255 ), 1.f );

    draw_list->AddCircleFilled( ImVec2( winpos.x + winsize.x * 0.5f, winpos.y + winsize.y * 0.5f ), config::options.esp.radar_icon_scale, ImColor( 255, 255, 255, 255 ) );

    auto render_image = [&winpos, &draw_list]( ID3D11ShaderResourceView *img, math::vector2 pos, sdk::color_t color ) {
        const auto image_scale = config::options.esp.radar_image_size;
        auto x_pos = ImVec2( winpos.x + pos.x() - image_scale, winpos.y + pos.y() - image_scale );
        auto y_pos = ImVec2( winpos.x + pos.x() + image_scale, winpos.y + pos.y() + image_scale );

        draw_list->AddImage( img, x_pos, y_pos, {}, { 1, 1 }, ImColor( color.r(), color.g(), color.b(), color.a() ) );
    };

    actor_manager->iterate_actors( [&]( cheats::actor_t *actor ) {
        if ( !should_draw_actor( actor ) ) {
            return;
        }

        if ( actor->component().relative_location.is_zero() ) {
            return;
        }

        auto texture = overlay::menu::get().textures()[ actor->icon_name() ];
        if ( !texture ) {
            return; // texture not found but object still wants to be drawn. maybe use a fallback image?
        }

        auto screen_pos = math::world_to_radar( actor->component().relative_location, actor_manager->local_pos(), actor_manager->local_angles(), winsize.x, config::options.esp.radar_zoom );
        if ( screen_pos.is_zero() ) {
            return;
        }

        auto clr = sdk::color_t::white();
        switch ( actor->tag() ) {
            case actor_tag_t::doors:
                clr = reinterpret_cast<cheats::door_t *>( actor )->inner().activated ? config::options.esp.door_open_color : config::options.esp.door_closed_color;
                break;

            case actor_tag_t::pallet:
                if ( reinterpret_cast<cheats::pallet_t *>( actor )->inner().pallet_state == sdk::apallet::pallet_state_t::broken ) {
                    return;
                }

                clr = ( reinterpret_cast<cheats::pallet_t *>( actor )->inner().pallet_state == sdk::apallet::pallet_state_t::up ) ? config::options.esp.pallet_up_color : config::options.esp.pallet_down_color;
                break;

            case actor_tag_t::totem: {
                auto totem = reinterpret_cast<cheats::totem_t *>( actor );
                if ( totem->inner().hex_perk_id.number == 0 ) {
                    clr = config::options.esp.dull_totem_color;
                } else {
                    clr = config::options.esp.hex_totem_color;
                }
                break;
            }

            case actor_tag_t::chest:
                clr = config::options.esp.chest_color;
                break;

            case actor_tag_t::killer_item:
                clr = config::options.esp.trap_color;
                break;

            case actor_tag_t::item:
                clr = sdk::color_t::white(); // should indicate rarity
                break;

            case actor_tag_t::generator: {
                auto gen = reinterpret_cast<cheats::generator_t *>( actor );

                float r = 0.f, g = 0.f, b = 0.f;
                if ( gen->inner().is_blocked || gen->inner().is_blocked_from_charging ) {
                    r = 0.5f;
                } else {
                    ImGui::ColorConvertHSVtoRGB( 0.47f, gen->inner().native_percent_complete, 1.f, r, g, b );
                }

                clr = sdk::color_t( r, g, b, 1.f );

                break;
            }

            case actor_tag_t::hook:
                if ( reinterpret_cast<cheats::hook_t *>( actor )->inner().is_sabotaged ||
                    reinterpret_cast<cheats::hook_t *>( actor )->inner().is_sacrificed ||
                    reinterpret_cast<cheats::hook_t *>( actor )->inner().is_survivor_struggling ) {
                    clr = sdk::color_t::red();
                }

                clr = config::options.esp.hook_color;
                break;

            case actor_tag_t::locker:
                clr = config::options.esp.locker_color;
                break;

            case actor_tag_t::hatch:
                clr = ( ( reinterpret_cast<cheats::hatch_t *>( actor )->inner().hatch_state == sdk::ahatch::hatch_state_t::default_open ) ||
                          ( reinterpret_cast<cheats::hatch_t *>( actor )->inner().hatch_state == sdk::ahatch::hatch_state_t::key_open ) ) ?
                    config::options.esp.hatch_open_color :
                    config::options.esp.hatch_closed_color;
                break;

            default:
                clr = sdk::color_t::white();
        }

        render_image( texture, screen_pos, clr );
    } );

    ImGui::End();
}

std::string cheats::esp_t::translate_name( std::string_view name )
{
    std::string ans;
    ans.reserve( name.size() );
}

bool cheats::esp_t::should_draw_actor( cheats::actor_t *actor )
{
    switch ( actor->tag() ) {
        case actor_tag_t::doors:
            return config::options.esp.filter_flags & config::doors;
        case actor_tag_t::pallet:
            return ( config::options.esp.filter_flags & config::pallets ) && ( ( reinterpret_cast<cheats::pallet_t *>( actor )->inner().pallet_state != sdk::apallet::pallet_state_t::broken ) && ( reinterpret_cast<cheats::pallet_t *>( actor )->inner().pallet_state != sdk::apallet::pallet_state_t::illusionary ) );
        case actor_tag_t::chest:
            return config::options.esp.filter_flags & config::chests && !( reinterpret_cast<cheats::chest_t *>( actor )->inner().has_been_searched );
        case actor_tag_t::killer_item:
            return config::options.esp.filter_flags & config::killers && config::options.esp.filter_flags & config::items;
        case actor_tag_t::item:
            return config::options.esp.filter_flags & config::items;
        case actor_tag_t::hook:
            return config::options.esp.filter_flags & config::hooks;
        case actor_tag_t::locker:
            return config::options.esp.filter_flags & config::lockers;
        case actor_tag_t::generator:
            return ( config::options.esp.filter_flags & config::generators ) && !( reinterpret_cast<cheats::generator_t *>( actor )->inner().is_repaired );
        case actor_tag_t::hatch:
            return config::options.esp.filter_flags & config::hatches;
        case actor_tag_t::totem:
            return ( config::options.esp.filter_flags & config::totems ) && !( reinterpret_cast<cheats::totem_t *>( actor )->inner().is_cleansed ) && ( !config::options.esp.hide_dull_totems || ( reinterpret_cast<cheats::totem_t *>( actor )->inner().hex_perk_id.number != 0 ) );
        case actor_tag_t::survivor:
            return ( config::options.esp.filter_flags & config::survivors ) && ( actor->base() != actor_manager->local_actor() );
        case actor_tag_t::killer:
            return ( config::options.esp.filter_flags & config::killers ) && ( actor->base() != actor_manager->local_actor() );

        default:
            return config::options.esp.debug_mode;
    }
}

std::vector<cheats::player_info_t> cheats::esp_t::get_players()
{
    std::vector<cheats::player_info_t> ans;

    auto get_info_from_state = [this]( std::string_view player_name, sdk::udbd_player_state &state ) -> std::optional<player_info_t> {
        std::wstring platform_account_id;
        platform_account_id.resize( static_cast<std::size_t>( state.platform_account_id.count * 2 ) );
        if ( !cheats::esp->m_process.read_ptr( reinterpret_cast<uintptr_t>( state.platform_account_id.data ), platform_account_id.data(), platform_account_id.size() ) ) {
            return std::nullopt;
        }

        std::vector<perk_t> perks;
        for ( int i = 0; i <= state.player_data.perk_ids.count; ++i ) {
            auto offset = uintptr_t( state.player_data.perk_ids.data ) + ( i * sizeof( sdk::fname ) );
            sdk::fname perk_name;
            m_process.read( offset, perk_name );

            if ( perk_name.number == 0 ) {
                continue;
            }

            auto name = cheats::actor_manager->get_name_for_id( perk_name.number );
            if ( perk_name_table.count(name) != 0 ) {
                perks.emplace_back( perk_name_table[name], 0 );
            } else {
                perks.emplace_back( name, 0 );
            }
        }

        std::vector<std::string> addons;
        auto player_data = state.game_role == 1 ? state.slasher_data : state.camper_data;
        for ( int i = 0; i < player_data.addon_ids.count; ++i ) {
            auto offset = uintptr_t( player_data.addon_ids.data ) + ( i * sizeof( sdk::fname ) );

            sdk::fname addon_name;
            m_process.read( offset, addon_name );

            auto addon_name_raw = cheats::actor_manager->get_name_for_id( addon_name.number );
            if ( addon_name_raw == "_EMPTY_" ) {
                continue;
            }

            addons.emplace_back( addon_name_raw );
        }

        auto power = cheats::actor_manager->get_name_for_id( player_data.power_id.number );
        if ( power == "_EMPTY_" ) {
            power = "";
        }

        auto offering = cheats::actor_manager->get_name_for_id( state.player_data.equipped_favor_id.number );
        if ( offering == "_EMPTY_" ) {
            offering = "";
        }

        return player_info_t { std::string( player_name ), platform_account_id, perks, addons, std::string( offering ), std::string( power ), state.game_role };
    };

    if ( !cheats::actor_manager->get_game_state().level_ready_to_play ) {
        auto player_array = cheats::actor_manager->get_game_state().player_array;
        for ( int i = 0; i < player_array.count; ++i ) {
            std::uintptr_t player_ptr;
            if ( !cheats::esp->m_process.read( uintptr_t( player_array.data ) + ( i * sizeof( uintptr_t ) ), player_ptr ) || !player_ptr ) {
                continue;
            }

            sdk::udbd_player_state state;
            if ( !cheats::esp->m_process.read( player_ptr, state ) ) {
                continue;
            }

            auto info = get_info_from_state( "unknown", state );
            if ( info ) {
                ans.emplace_back( *info );
            }
        }

        return ans;
    }

    cheats::actor_manager->iterate_actors( [this, &ans, &get_info_from_state]( cheats::actor_t *actor ) {
        if ( !actor || ( ( actor->tag() != cheats::actor_tag_t::survivor ) && ( actor->tag() != cheats::actor_tag_t::killer ) ) ) {
            return;
        }

        sdk::udbd_player_state state;
        bool read_successfuly = cheats::esp->m_process.read( reinterpret_cast<std::uintptr_t>(
                                                                 ( actor->tag() == cheats::actor_tag_t::survivor ) ?
                                                                     actor->as<cheats::survivor_t>()->inner().state :
                                                                     actor->as<cheats::killer_t>()->inner().state ),
            state );
        if ( !read_successfuly ) {
            return;
        }

        auto info = get_info_from_state( actor->name(), state );
        if ( info ) {
            ans.emplace_back( *info );
        }

        /*std::wstring platform_account_id;
        platform_account_id.resize( static_cast<std::size_t>( state.platform_account_id.count * 2 ) );
        if ( !cheats::esp->m_process.read_ptr( reinterpret_cast<uintptr_t>( state.platform_account_id.data ), platform_account_id.data(), platform_account_id.size() ) ) {
            return;
        }

        sdk::uperk_manager manager;
        read_successfuly = cheats::esp->m_process.read(
            reinterpret_cast<std::uintptr_t>(
                ( actor->tag() == cheats::actor_tag_t::survivor ) ?
                    actor->as<cheats::survivor_t>()->inner().perk_manager :
                    actor->as<cheats::killer_t>()->inner().perk_manager ),
            manager );
        if ( !read_successfuly ) {
            return;
        }

        sdk::uperk_collection_component perk_component;
        cheats::esp->m_process.read( reinterpret_cast<std::uintptr_t>( manager.perks ), perk_component );

        std::vector<perk_t> perks;
        for ( int i = 0; i < perk_component._array.count; ++i ) {
            std::uintptr_t perk_ptr;
            if ( !cheats::esp->m_process.read( uintptr_t( perk_component._array.data ) + ( i * sizeof( std::uintptr_t ) ), perk_ptr ) ) {
                continue;
            }

            sdk::uperk perk;
            if ( !cheats::esp->m_process.read( perk_ptr, perk ) ) {
                continue;
            }

            auto raw_name = cheats::actor_manager->get_name_for_id( perk.id.number );
            perks.emplace_back( raw_name, perk.perk_level );
        }

        std::vector<std::string> addons;
        auto player_data = state.game_role == 1 ? state.slasher_data : state.camper_data;
        for ( int i = 0; i < player_data.addon_ids.count; ++i ) {
            auto offset = uintptr_t( player_data.addon_ids.data ) + ( i * sizeof( sdk::fname ) );

            sdk::fname addon_name;
            m_process.read( offset, addon_name );

            auto addon_name_raw = cheats::actor_manager->get_name_for_id( addon_name.number );
            if ( addon_name_raw == "_EMPTY_" ) {
                continue;
            }

            addons.emplace_back( addon_name_raw );
        }

        auto power = cheats::actor_manager->get_name_for_id( player_data.power_id.number );
        if ( power == "_EMPTY_" ) {
            power = "";
        }

        auto offering = cheats::actor_manager->get_name_for_id( state.player_data.equipped_favor_id.number );
        if ( offering == "_EMPTY_" ) {
            offering = "";
        }

        ans.emplace_back( actor->name(), platform_account_id, perks, addons, std::string( offering ), std::string( power ), state.game_role );*/
    } );

    return ans;
}