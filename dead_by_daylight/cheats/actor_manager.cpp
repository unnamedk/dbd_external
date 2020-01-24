#include "actor_manager.hpp"
#include "offsets.hpp"
#include "../config/options.hpp"
#include <thread>

std::optional<cheats::actor_manager_t> cheats::actor_manager;

enum string_comparision : std::uint32_t
{
    equals = 1 << 0, // only if the entire string matches
    contains = 1 << 1, // only if string A contains substring B
    starts_with = 1 << 2, // only if string A starts with string B

    case_insensitive = 1 << 3 // case insensitive compare (default method is case sensitive)
};

bool str_compare( std::string_view a, std::string_view b, std::uint32_t comparision = contains | case_insensitive )
{
    bool case_insensitive = comparision & string_comparision::case_insensitive;
    auto default_compare = [case_insensitive]( auto a, auto b ) {
        return case_insensitive ? ( std::tolower( static_cast<int>( a ) ) == std::tolower( static_cast<int>( b ) ) ) : ( a == b );
    };

    if ( comparision & contains ) {
        return std::search( a.cbegin(), a.cend(), b.cbegin(), b.cend(), default_compare ) != a.cend();
    }

    else if ( comparision & starts_with ) {
        return ( case_insensitive ? _strnicmp( a.data(), b.data(), b.size() ) : strncmp( a.data(), b.data(), b.size() ) ) == 0;
    }

    return std::equal( a.cbegin(), a.cend(), b.cbegin(), b.cend(), default_compare );
}

void cheats::actor_manager_t::run_thread()
{
    this->update_names();
    std::vector<std::uintptr_t> m_current_ptrs;
    std::vector<std::uintptr_t> m_old_ptrs;

    while ( config::options.misc.should_run_threads ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

        sdk::uworld world;
        if ( std::uintptr_t world_ptr = 0ull; !this->m_process.read( this->m_process.base_address() + offsets->uworld, world_ptr ) || !world_ptr || !this->m_process.read( world_ptr, world ) ) {
            continue;
        }

        this->m_process.read( reinterpret_cast<std::uintptr_t>(world.game_state), game_state );

        sdk::ulevel level;
        if ( !this->m_process.read( reinterpret_cast<std::uintptr_t>( world.level ), level ) ) {
            continue;
        }

        sdk::ugameinstance game_instance;
        if ( !this->m_process.read( reinterpret_cast<std::uintptr_t>( world.game_instance ), game_instance ) ) {
            continue;
        }

        if ( std::uintptr_t local_player_ptr = 0ull;
             !this->m_process.read( reinterpret_cast<std::uintptr_t>( game_instance.local_players.data ), local_player_ptr ) ||
             !this->m_process.read( local_player_ptr, m_local_player ) ) {
            continue;
        }

        m_current_ptrs.resize( level.actor_list.count, {} );
        m_actors.resize( level.actor_list.count );
        if ( !this->m_process.read_ptr( reinterpret_cast<std::uintptr_t>( level.actor_list.data ), m_current_ptrs.data(), level.actor_list.count * sizeof( std::uintptr_t ) ) ) {
            continue;
        }

        // fallback position if there's no PlayerCameraManager yet
        bool has_camera_manager = false;

        std::unique_lock lock { m_array_lock };
        for ( auto i = 0u; i < m_current_ptrs.size(); ++i ) {
            auto ptr = m_current_ptrs[ i ];
            if ( !ptr ) {
                continue;
            }

            sdk::aactor actor;
            if ( !this->m_process.read( ptr, actor ) || ( actor.name.number == 0 ) ) {
                continue;
            }

            auto name = get_name_for_id( actor.name.number );
            if ( name.empty() ) {
                continue;
            }

            if ( name == "DBDPlayerCameraManager" ) {
                sdk::aplayer_camera_manager camera_mgr;
                if ( sdk::aactor spected_actor; this->m_process.read( ptr, camera_mgr ) && this->m_process.read( reinterpret_cast<std::uintptr_t>( camera_mgr.view_target.target ), spected_actor ) ) {
                    sdk::uscenecomponent root_component;
                    if ( this->m_process.read( reinterpret_cast<std::uintptr_t>( spected_actor.root_component ), root_component ) ) {
                        has_camera_manager = true;
                        this->m_local_pos = root_component.relative_location;
                        this->m_local_velocity = root_component.velocity;
                        this->m_local_actor = reinterpret_cast<std::uintptr_t>( camera_mgr.view_target.target );
                    }
                }

                sdk::uscenecomponent actor_component;
                if ( this->m_process.read( reinterpret_cast<std::uintptr_t>( actor.root_component ), actor_component ) ) {
                    this->m_local_angles = actor_component.relative_rotation;
                }
            }

            auto info = this->parse_actor_info( name );

            switch ( info.type ) {
                case actor_tag_t::survivor: {
                    m_actors[ i ] = std::move( std::unique_ptr<cheats::survivor_t>( new cheats::survivor_t( ptr, actor_tag_t::survivor, info.pretty_name, info.name_to_icon(), name ) ) );
                    break;
                }
                case actor_tag_t::killer: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::killer_t>( ptr, actor_tag_t::killer, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                case actor_tag_t::killer_item: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::trap_t>( ptr, actor_tag_t::killer_item, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }

                case actor_tag_t::item: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::item_t>( ptr, actor_tag_t::item, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                case actor_tag_t::generator: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::generator_t>( ptr, actor_tag_t::generator, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                case actor_tag_t::hatch: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::hatch_t>( ptr, actor_tag_t::hatch, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                case actor_tag_t::doors: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::door_t>( ptr, actor_tag_t::doors, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                case actor_tag_t::totem: {
                    m_actors[ i ] = ( std::move( std::make_unique<cheats::totem_t>( ptr, actor_tag_t::totem, info.pretty_name, info.name_to_icon(), name ) ) );
                    break;
                }
                case actor_tag_t::locker: {
                    m_actors[ i ] = ( std::move( std::make_unique<cheats::locker_t>( ptr, actor_tag_t::locker, info.pretty_name, info.name_to_icon(), name ) ) );
                    break;
                }
                case actor_tag_t::hook: {
                    m_actors[ i ] = ( std::move( std::make_unique<cheats::hook_t>( ptr, actor_tag_t::hook, info.pretty_name, info.name_to_icon(), name ) ) );
                    break;
                }
                case actor_tag_t::pallet: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::pallet_t>( ptr, actor_tag_t::pallet, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                case actor_tag_t::chest: {
                    m_actors[ i ] = std::move( std::make_unique<cheats::chest_t>( ptr, actor_tag_t::chest, info.pretty_name, info.name_to_icon(), name ) );
                    break;
                }
                default: {
                    if ( config::options.esp.debug_mode ) {
                        m_actors[ i ] = ( std::move( std::make_unique<cheats::default_t>( ptr, actor_tag_t::unknown, std::string( name ), "", name ) ) );
                    }
                }
            }

            if ( m_actors[ i ] ) {
                m_actors[ i ]->update( this->m_process );
            }
        }

        if ( !has_camera_manager ) {
            this->m_local_pos = m_local_player.position;
        }
    }
}

std::string_view cheats::actor_manager_t::get_name_for_id( std::uint32_t id )
{
    if ( id >= this->m_names.size() ) {
        return "";
    }

    std::unique_lock lock { this->m_name_lock };
    return this->m_names[ id ];
}

void cheats::actor_manager_t::update_names()
{
    std::uintptr_t gnames = 0ull;
    if ( !m_process.read( m_process.base_address() + offsets->gnames, gnames ) ) {
        return;
    }

    std::vector<std::uintptr_t> chunk_list;
    for ( int i = 0; i < 30; ++i ) {
        uintptr_t chunk = 0ull;
        if ( !m_process.read( gnames + ( sizeof( std::uintptr_t ) * i ), chunk ) ) {
            continue;
        }

        if ( chunk != 0ull ) {
            chunk_list.push_back( chunk );
        }
    }

    std::unique_lock lock { this->m_name_lock };
    if ( !this->m_names.empty() ) {
        this->m_names.clear();
    }

    constexpr auto chunk_size = 16'384;
    for ( uintptr_t chunk : chunk_list ) {
        auto chunk_array = std::make_unique<std::uintptr_t[]>( chunk_size );
        if ( !m_process.read_ptr( chunk, chunk_array.get(), chunk_size * sizeof( std::uintptr_t ) ) ) {
            continue;
        }

        for ( auto i = 0; i < chunk_size; ++i ) {
            sdk::fname_entry name_object;
            if ( !chunk_array[ i ] || !( m_process.read( chunk_array[ i ], name_object ) ) ) {
                continue;
            }

            this->m_names[ name_object.index / 2 ] = name_object.ansi_name;
        }
    }
}

cheats::actor_info cheats::actor_manager_t::parse_actor_info( std::string_view name )
{
    if ( str_compare( name, "BP_BearTrap_001_C", equals ) ||
        str_compare( name, "BearTrap_C", equals ) ||
        str_compare( name, "GroundPortal_C", equals ) ||
        str_compare( name, "DreamSnare_C", equals ) ||
        str_compare( name, "BP_ReverseBearTrapRemover", starts_with )) {
        return { "Trap", actor_tag_t::killer_item };
    }

    if ( str_compare( name, "BP_PhantomTrap_C", equals ) ) {
        return { "Phantom Trap", actor_tag_t::killer_item };
    }

    if ( str_compare( name, "BP_Hatch01_C", equals ) ||
        str_compare( name, "HatchSpawner_BP_TL", equals ) ) {
        return { "Hatch", actor_tag_t::hatch };
    }

    if ( str_compare( name, "BP_EscapeBlocker_C", equals ) ) {
        return { "Door", actor_tag_t::doors };
    }

    if ( str_compare( name, "BP_TotemBase_C", equals ) ) {
        return { "Totem", actor_tag_t::totem };
    }

    if ( str_compare( name, "Bookshelf_C", contains ) ) {
        return { "Pallet", actor_tag_t::pallet };
    }

    if ( str_compare( name, "Searchable_BP_C", equals ) ) {
        return { "Chest", actor_tag_t::chest };
    }

    if ( str_compare( name, "ClosetStandard_C", equals ) ) {
        return { "Locker", actor_tag_t::locker };
    }

    if ( const auto generator_index = name.find( "Generator" ); generator_index != std::string::npos ) {
        if ( generator_index != 0 ) {
            return { "", actor_tag_t::unknown };
        }

        std::string_view gen { name };
        gen.remove_prefix( 9 ); // skip "Generator" part

        if ( str_compare( gen, "Hospital_C", starts_with ) ||
            str_compare( gen, "Standard_C", starts_with ) ||
            str_compare( gen, "Suburbs_C", starts_with ) ||
            str_compare( gen, "LunarIndoors_C", starts_with ) ||
            str_compare( gen, "LunarOutdoors_C", starts_with ) ||
            str_compare( gen, "_Halloween_2019_C", starts_with ) ||
            str_compare( gen, "Standard_Anniversary2019_C", starts_with ) ||
            str_compare( gen, "Suburbs_Anniversary2019_C", starts_with ) ||
            str_compare( gen, "Suburbs_Anniversary2019_C", starts_with ) ||
            str_compare( gen, "SummerIndoors_C", starts_with ) ||
            str_compare( gen, "SummerOutdoors_C", starts_with ) ||
            str_compare( gen, "_Indoors_Halloween_2019_C", starts_with ) ) {
            return { "Generator", actor_tag_t::generator };
        }

        return { "", actor_tag_t::unknown };
    }

    if ( str_compare( name, "BP_MeatLockerBasement0", starts_with ) && ( name.size() < 27 ) ) {
        return { "Hook", actor_tag_t::hook };
    }

    if ( const auto generator_index = name.find( "BP_SmallMeatLocker" ); generator_index != std::string::npos ) {
        if ( generator_index != 0 ) {
            return { "", actor_tag_t::unknown };
        }

        std::string_view hook { name };
        hook.remove_prefix( 18 ); // skip "BP_SmallMeatLocker" part

        if ( str_compare( hook, "_C", starts_with ) ||
            str_compare( hook, "_Anniversary2019_C", starts_with ) ||
            str_compare( hook, "_Banshee_C", starts_with ) ||
            str_compare( hook, "_Chuckles_C", starts_with ) ||
            str_compare( hook, "_Doctor_C", starts_with ) ||
            str_compare( hook, "_Halloween_C", starts_with ) ||
            str_compare( hook, "_Halloween_2019_C", starts_with ) ||
            str_compare( hook, "_HillBilly_C", starts_with ) ||
            str_compare( hook, "_Huntress_C", starts_with ) ||
            str_compare( hook, "_Legion_C", starts_with ) ||
            str_compare( hook, "_Lunar_C", starts_with ) ||
            str_compare( hook, "_Nurse_C", starts_with ) ||
            str_compare( hook, "_Pig_C", starts_with ) ||
            str_compare( hook, "_Qatar_C", starts_with ) ||
            str_compare( hook, "_Sandman_C", starts_with ) ||
            str_compare( hook, "_Spirit_C", starts_with ) ||
            str_compare( hook, "_Summer_C", starts_with ) ||
            str_compare( hook, "_Winter2018_C", starts_with ) ||
            str_compare( hook, "_Witch_C", starts_with ) ||
            str_compare( hook, "_MM_C", starts_with ) ) {
            return { "Hook", actor_tag_t::hook };
        }

        return { "", actor_tag_t::unknown };
    }

    if ( str_compare( name, "BP_Item_Camper_DullKey_C", equals ) ||
        str_compare( name, "BP_Item_Camper_Key_C" ) ) {
        return { "Key", actor_tag_t::item };
    }

    if ( str_compare( name, "BP_Medkit_00", starts_with ) ) {
        return { "Medkit", actor_tag_t::item };
    }

    if ( str_compare( name, "BP_Flashlight_00", starts_with ) ) {
        return { "Flashlight", actor_tag_t::item };
    }

    if ( str_compare( name, "BP_Toolbox_00", starts_with ) ) {
        return { "Toolbox", actor_tag_t::item };
    }

    // survivors
    {
        if ( str_compare( name, "BP_CamperFemale01_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale01_C", equals ) ) {
            return { "Meg Thomas", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale02_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale02_C", equals ) ) {
            return { "Claudette Morel", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale03_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale03_C", equals ) ) {
            return { "Nea Karlsson", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale04_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale04_C", equals ) ) {
            return { "Laurie Strode", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale05_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale05_C", equals ) ) {
            return { "Feng Min", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale06_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale06_C", equals ) ) {
            return { "Kate Denson", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale07_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale07_C", equals ) ) {
            return { "Jane Romero", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale08_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale08_C", equals ) ) {
            return { "Nancy Wheeler", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperFemale09_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperFemale09_C", equals ) ) {
            return { "Yui Kimura", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale01_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale01_C", equals ) ) {
            return { "Dwight Fairfield", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale02_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale02_C", equals ) ) {
            return { "Jake Park", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale03_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale03_C", equals ) ) {
            return { "Ace Visconti", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale04_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale04_C", equals ) ) {
            return { "Bill Overbeck", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale05_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale05_C", equals ) ) {
            return { "David King", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale06_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale06_C", equals ) ) {
            return { "Quentin Smith", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale07_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale07_C", equals ) ) {
            return { "Detective Tapp", actor_tag_t::survivor };
        }

        if ( str_compare( name, "BP_CamperMale08_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale08_C", equals ) ) {
            return { "Adam Francis", actor_tag_t::survivor };
        }
        if ( str_compare( name, "BP_CamperMale09_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale09_C", equals ) ) {
            return { "Jeff Johansen", actor_tag_t::survivor };
        }
        if ( str_compare( name, "BP_CamperMale10_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale10_C", equals ) ) {
            return { "Ash Williams", actor_tag_t::survivor };
        }
        if ( str_compare( name, "BP_CamperMale11_Character_C", equals ) ||
            str_compare( name, "BP_Menu_CamperMale11_C", equals ) ) {
            return { "Steve Harrington", actor_tag_t::survivor };
        }
    }

    // killers
    {
        if ( str_compare( name, "BP_Slasher_Character_01_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher01_C", equals ) ) {
            return { "Trapper", actor_tag_t::killer };
        }

        if ( str_compare( name, "BP_Slasher_Character_02_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher02_C", equals ) ) {
            return { "Wraith", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_03_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher03_C", equals ) ) {
            return { "Hillbilly", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_04_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher04_C", equals ) ) {
            return { "Nurse", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_05_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher05_C", equals ) ) {
            return { "Hag", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_06_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher06_C", equals ) ) {
            return { "Myers", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_07_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher07_C", equals ) ) {
            return { "Doctor", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_08_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher08_C", equals ) ) {
            return { "Huntress", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_09_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher09_C", equals ) ) {
            return { "Bubba", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_10_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher10_C", equals ) ) {
            return { "Freddy", actor_tag_t::killer };
        }

        if ( str_compare( name, "BP_Slasher_Character_11_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher11_C", equals ) ) {
            return { "Pig", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_12_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher12_C", equals ) ) {
            return { "Clown", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_13_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher13_C", equals ) ) {
            return { "Spirit", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_14_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher14_C", equals ) ) {
            return { "Legion", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_15_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher15_C", equals ) ) {
            return { "Plague", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_16_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher16_C", equals ) ) {
            return { "Ghostface", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_17_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher17_C", equals ) ) {
            return { "Demogorgon", actor_tag_t::killer };
        }
        if ( str_compare( name, "BP_Slasher_Character_18_C", equals ) ||
            str_compare( name, "BP_Menu_Slasher18_C", equals ) ) {
            return { "Oni", actor_tag_t::killer };
        }
    }

    return { "", actor_tag_t::unknown };
}
