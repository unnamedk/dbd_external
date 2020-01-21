#include "config.hpp"
#include "options.hpp"

#include <shlobj.h>
#pragma comment( lib, "shell32.lib" )

using namespace config;
std::optional<config::settings_manager> config::config_system;

void settings_manager::init()
{
    // Setup config values
    this->populate_settings();

    wchar_t *path_to_docs = nullptr;

    // Get current user's "Documents" folder
    SHGetKnownFolderPath( FOLDERID_Documents, 0, nullptr, &path_to_docs );

    // Folder we will be operating on
    ( configs_path = path_to_docs ) /= ( "Cheat Settings" );

    // If the cheat folder doesn't exist, create it
    if ( !std::filesystem::exists( configs_path ) ) {
        std::filesystem::create_directory( configs_path );
    }

    // Use default config file
    auto default_cfg = configs_path / ( "dbd_default_settings.json" );
    cfg.set_config_file( default_cfg.string() );

    // Free the allocated memory
    CoTaskMemFree( path_to_docs );

    // Load settings from the file
    this->load();
}

void config::settings_manager::load()
{
    const auto load_for = [this]( auto &vec ) {
        for ( auto &opt : vec ) {
            opt.variable = this->cfg.get_or_insert( opt.default_value, opt.path );
        }
    };

    options.esp.priority_table = this->cfg.get_or_insert( options.esp.priority_table, "esp.priority_table" );

    load_for( bools );
    load_for( ints );
    load_for( floats );
    load_for( strings );
    load_for( colors );
}

void config::settings_manager::save()
{
    const auto save_for = [this]( auto &vec ) {
        for ( auto &opt : vec ) {
            this->cfg.put( opt.variable, opt.path );
        }
    };
    this->cfg.put( options.esp.priority_table, "esp.priority_table" );
    save_for( bools );
    save_for( ints );
    save_for( floats );
    save_for( strings );
    save_for( colors );

    cfg.save_to_disk();
}

void config::settings_manager::populate_settings()
{
    setup_value( "esp.enabled", config::options.esp.enabled, true );
    setup_value( "esp.filter_flags", config::options.esp.filter_flags, ( killers | survivors | totems | hatches ) );
    setup_value( "esp.radar_enabled", config::options.esp.radar, true );
    setup_value( "esp.radar_preview_icons", config::options.esp.radar_preview_icons, false );
    setup_value( "esp.radar_icon_scale", config::options.esp.radar_icon_scale, 4.5f );
    setup_value( "esp.radar_image_size", config::options.esp.radar_image_size, 32.f );
    setup_value( "esp.radar_zoom", config::options.esp.radar_zoom, config::options.esp.radar_icon_scale );
    setup_value( "esp.hide_dull_totems", config::options.esp.hide_dull_totems, false );

    setup_value( "esp.dull_totem_color", config::options.esp.dull_totem_color, config::options.esp.dull_totem_color );
    setup_value( "esp.hex_totem_color", config::options.esp.hex_totem_color, config::options.esp.hex_totem_color );
    setup_value( "esp.hatch_closed_color", config::options.esp.hatch_closed_color, config::options.esp.hatch_closed_color );
    setup_value( "esp.hatch_open_color", config::options.esp.hatch_open_color, config::options.esp.hatch_open_color );
    setup_value( "esp.door_closed_color", config::options.esp.door_closed_color, config::options.esp.door_closed_color );
    setup_value( "esp.door_open_color", config::options.esp.door_open_color, config::options.esp.door_open_color );
    setup_value( "esp.pallet_up_color", config::options.esp.pallet_up_color, config::options.esp.pallet_up_color );
    setup_value( "esp.pallet_down_color", config::options.esp.pallet_down_color, config::options.esp.pallet_down_color );
    setup_value( "esp.hook_color", config::options.esp.hook_color, config::options.esp.hook_color );
    setup_value( "esp.chest_color", config::options.esp.chest_color, config::options.esp.chest_color );
    setup_value( "esp.locker_color", config::options.esp.locker_color, config::options.esp.locker_color );
    setup_value( "esp.trap_color", config::options.esp.trap_color, config::options.esp.trap_color );

    setup_value( "misc.autoskillcheck", config::options.misc.autoskillcheck, false );
    setup_value( "misc.autoskillcheck_key", config::options.misc.autoskillcheck_key, VK_XBUTTON1 );
    setup_value( "misc.autopallet", config::options.misc.autopallet, false );
    setup_value( "misc.autopallet_key", config::options.misc.autopallet_key, VK_MENU );
}