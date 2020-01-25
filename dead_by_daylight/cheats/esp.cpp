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
#include "meta.hpp"

std::optional<cheats::esp_t> cheats::esp;

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

            case actor_tag_t::killer_item: {
                auto trap = actor->as<cheats::trap_t>();
                if ( trap->is_rbt_remover() && trap->keys ) {
                    if ( !trap->keys->empty() ) {
                        names.emplace_back( fmt::format( "Trap (keys: {})", fmt::join( *trap->keys, ", " ) ), actor->priority(), actor->component().relative_location );
                    } else {
                        names.emplace_back( "Trap (no keys)", actor->priority(), actor->component().relative_location );
                    }
                } else {
                    names.emplace_back( "Trap", actor->priority(), actor->component().relative_location );
                }

                break;
            }

            case actor_tag_t::survivor: {
                auto survivor = reinterpret_cast<cheats::survivor_t *>( actor );
                if ( !survivor->inner().health_component || ( survivor->health_component().current_health_state_count > 2 ) || ( survivor->health_component().current_health_state_count < -1 ) ) {
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
    ImGui::Columns( 6 );
    ImGui::Text( "character" );
    ImGui::NextColumn();

    ImGui::Text( "profile" );
    ImGui::NextColumn();

    ImGui::Text( "perks" );
    ImGui::NextColumn();

    ImGui::Text( "power/item" );
    ImGui::NextColumn();

    ImGui::Text( "addons" );
    ImGui::NextColumn();

    ImGui::Text( "offering" );
    ImGui::NextColumn();

    auto &tex = overlay::menu::get().textures();

    const auto desired_icon_size = ImVec2( 40, 40 );
    auto describe_item = [&tex, &desired_icon_size]( std::string_view item_category, const std::string &name, const std::string &item_id ) {
        const auto has_image = tex.count( item_id ) != 0;

        if ( has_image ) {
            const auto has_entry = meta::item_entries.count( item_id ) != 0;
            if ( has_entry ) {
                const auto &entry = meta::item_entries[ item_id ];

                const auto tooltip = fmt::format( "{}: {}\n\nDescription: {}", item_category, entry.name(), entry.description() );

                auto color = sdk::color_t::brown();
                switch ( entry.rarity() ) {
                    case meta::item_rarity::event:
                        color = sdk::color_t::orange();
                        break;
                    case meta::item_rarity::uncommon:
                        color = sdk::color_t::yellow();
                        break;
                    case meta::item_rarity::rare:
                        color = sdk::color_t::spring_green();
                        break;
                    case meta::item_rarity::very_rare:
                        color = sdk::color_t::purple();
                        break;
                    case meta::item_rarity::ultra_rare:
                        color = sdk::color_t::magenta();
                        break;
                    default:
                        color = sdk::color_t::white();
                        break;
                }
                  
                ImGui::Image( tex[ item_id ], desired_icon_size, {}, { 1, 1 }, ImVec4( color.clamped_r(), color.clamped_g(), color.clamped_b(), 1.f ), {} );
                set_tooltip( std::data( tooltip ) );
            } else {
                ImGui::Image( tex[ item_id ], desired_icon_size );
                set_tooltip( name.data() );
            }
        } else {
            ImGui::Text( " %s ", name.data() );
        }
    };

    int prop_id = 0;

    auto players = get_players();
    for ( auto &p : players ) {
        ImGui::Image( tex[ p.icon_name ], desired_icon_size );
        ImGui::NextColumn();

        auto plat_id = nt::utils::narrow( p.platform_id );
        auto url = fmt::format( "https://steamcommunity.com/profiles/{}", plat_id );

        ImGui::PushID( prop_id++ );
        if ( ImGui::ImageButton( tex[ "paste" ], desired_icon_size ) ) {
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
        set_tooltip( url.data() );

        ImGui::NextColumn();

        auto str_to_icon = []( std::string s ) {
            std::string ans;
            ans.reserve( s.size() );

            for ( auto c : s ) {
                if ( ::isalnum( static_cast<int>( c ) ) ) {
                    ans.push_back( static_cast<char>( ::tolower( static_cast<int>( c ) ) ) );
                }
            }

            return ans;
        };

        for ( int i = 0; i < p.perks.size(); ++i ) {
            const auto item_id = str_to_icon( p.perks[ i ].name() );
            describe_item( "Perk", p.perks[ i ].name(), item_id );
            if ( i != ( p.perks.size() - 1 ) ) {
                ImGui::SameLine();
            }
        }
        ImGui::NextColumn();

        {
            const auto item_id = str_to_icon( p.power );
            describe_item( "Item", p.power, item_id );
        }
        ImGui::NextColumn();

        for ( int i = 0; i < p.addons.size(); ++i ) {
            const auto item_id = str_to_icon( p.addons[ i ] );
            describe_item( "Add-on", p.addons[ i ], item_id );
            if ( i != ( p.addons.size() - 1 ) ) {
                ImGui::SameLine();
            }
        }
        ImGui::NextColumn();

        {
            const auto item_id = str_to_icon( p.offering );
            describe_item( "Offering", p.offering, item_id );
        }
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

    auto gamestate = cheats::actor_manager->get_game_state();

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

        auto screen_pos = math::world_to_radar( actor->component().relative_location, actor_manager->local_pos(), actor_manager->local_angles(), static_cast<int>( winsize.x ), config::options.esp.radar_zoom );
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

            case actor_tag_t::killer_item: {
                auto totem = actor->as<cheats::trap_t>();
                if ( totem->is_rbt_remover() && totem->keys && totem->keys->empty() ) {
                    clr = config::options.esp.dull_totem_color;
                } else {
                    clr = config::options.esp.trap_color;
                }

                break;
            }
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

    auto get_info_from_state = [this]( std::string_view player_name, std::optional<std::string> icon_name, sdk::udbd_player_state &state ) -> std::optional<player_info_t> {
        auto try_translate = []( std::string_view raw_name, bool fallback_translation = false, std::size_t skip_first = 0 ) -> std::string {
            if ( meta::item_translation_table.count( raw_name ) != 0 ) {
                return meta::item_translation_table[ raw_name ];
            }

            // translate CamelCase and names such as Sprint_Burst to Sprint Burst
            if ( fallback_translation ) {
                if ( skip_first != std::string::npos ) {
                    raw_name.remove_prefix( skip_first );
                }

                std::string ans {};
                ans.reserve( raw_name.size() + 1 );

                char prev_c = '\0';
                for ( auto c : raw_name ) {
                    if ( c == '_' ) {
                        ans.push_back( ' ' );
                    }

                    else if ( ::isupper( static_cast<int>( c ) ) && ::islower( prev_c ) ) {
                        ans.push_back( ' ' );
                        ans.push_back( c );
                    }

                    else {
                        ans.push_back( c );
                    }
                    prev_c = c;
                }

                return ans;
            }

            return std::string( raw_name );
        };

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
            perks.emplace_back( try_translate( name, true ), 0 );
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
            auto addon = try_translate( addon_name_raw, true, addon_name_raw.find_last_of( '_' ) );

            addons.emplace_back( addon );
        }

        auto power_raw = cheats::actor_manager->get_name_for_id( player_data.power_id.number );
        if ( power_raw == "_EMPTY_" ) {
            power_raw = "";
        }

        auto power = try_translate( power_raw, true, power_raw.find_last_of( '_' ) );

        auto offering_raw = cheats::actor_manager->get_name_for_id( state.player_data.equipped_favor_id.number );
        if ( offering_raw == "_EMPTY_" ) {
            offering_raw = "";
        }

        auto offering = try_translate( offering_raw, true );

        std::string icon;
        if ( icon_name ) {
            icon = *icon_name;
        } else {
            icon = state.game_role == 1 ? "killer" : "survivor";
        }

        return player_info_t { std::string( player_name ), icon, platform_account_id, perks, addons, std::string( offering ), std::string( power ), state.game_role };
    };

    if ( !cheats::actor_manager->get_game_state().level_ready_to_play ) {
        auto player_array = cheats::actor_manager->get_game_state().player_array;
        if ( !player_array.data || !player_array.count ) {
            return ans;
        }

        for ( int i = 0; i < player_array.count; ++i ) {
            std::uintptr_t player_ptr;
            if ( !cheats::esp->m_process.read( uintptr_t( player_array.data ) + ( i * sizeof( uintptr_t ) ), player_ptr ) || !player_ptr ) {
                continue;
            }

            sdk::udbd_player_state state;
            if ( !cheats::esp->m_process.read( player_ptr, state ) ) {
                continue;
            }

            auto info = get_info_from_state( "unknown", std::nullopt, state );
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

        auto info = get_info_from_state( actor->name(), actor->icon_name(), state );
        if ( info ) {
            ans.emplace_back( *info );
        }
    } );

    return ans;
}