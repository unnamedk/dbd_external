#include "config.hpp"
#include "options.hpp"

#include <streambuf>
#include <xorstr.hpp>

#include <shlobj.h>
#pragma comment( lib, "shell32.lib" )

using namespace config;
std::optional<config::settings_manager> config::config_system;

void settings_manager::init()
{
    // Setup config values
    this->populate_settings();

    wchar_t *path_to_docs = nullptr;

    // Get current user's xorstr_("Documents") folder
    SHGetKnownFolderPath( FOLDERID_Documents, 0, nullptr, &path_to_docs );

    // Folder we will be operating on
    ( configs_path = path_to_docs ) /= ( xorstr_("Cheat Settings") );

    // If the cheat folder doesn't exist, create it
    if ( !fs::exists( configs_path ) ) {
        fs::create_directory( configs_path );
    }

    // Use default config file
    auto default_cfg = configs_path / ( xorstr_("dbd_default_settings.json") );
    main_cfg.set_config_file( default_cfg.string() );

    // handle scripts
    this->scripts_path = configs_path / xorstr_("dbd_scripts");
    if ( !fs::exists( this->scripts_path ) ) {
        fs::create_directory( this->scripts_path );
    }

    this->populate_scripts();

    // Free the allocated memory
    CoTaskMemFree( path_to_docs );

    // Load settings from the file
    this->load();
}

void config::settings_manager::load()
{
    const auto load_for = [this]( auto &vec ) {
        for ( auto &opt : vec ) {
            opt.variable = this->main_cfg.get_or_insert( opt.default_value, opt.path );
        }
    };

    options.esp.priority_table = this->main_cfg.get_or_insert( options.esp.priority_table, xorstr_("esp.priority_table") );

    load_for( bools );
    load_for( ints );
    load_for( floats );
    load_for( strings );
    load_for( colors );

    // TODO: need a mutex here
    config::options.scripts.script_data.clear();
    this->populate_scripts();
}

void config::settings_manager::save()
{
    const auto save_for = [this]( auto &vec ) {
        for ( auto &opt : vec ) {
            this->main_cfg.put( opt.variable, opt.path );
        }
    };
    this->main_cfg.put( options.esp.priority_table, xorstr_("esp.priority_table") );
    save_for( bools );
    save_for( ints );
    save_for( floats );
    save_for( strings );
    save_for( colors );

    main_cfg.save_to_disk();
    for ( auto &s : config::options.scripts.script_data ) {
        std::ofstream ofs( this->scripts_path / s.name(), std::ios::out | std::ios::trunc );
        ofs << s.data();
    }
}

void config::settings_manager::populate_settings()
{
    setup_value( xorstr_("esp.enabled"), config::options.esp.enabled, true );
    setup_value( xorstr_("esp.filter_flags"), config::options.esp.filter_flags, ( killers | survivors | totems | hatches ) );
    setup_value( xorstr_("esp.radar_enabled"), config::options.esp.radar, true );
    setup_value( xorstr_("esp.radar_preview_icons"), config::options.esp.radar_preview_icons, false );
    setup_value( xorstr_("esp.radar_icon_scale"), config::options.esp.radar_icon_scale, 4.5f );
    setup_value( xorstr_("esp.radar_image_size"), config::options.esp.radar_image_size, 32.f );
    setup_value( xorstr_("esp.radar_zoom"), config::options.esp.radar_zoom, config::options.esp.radar_icon_scale );
    setup_value( xorstr_("esp.hide_dull_totems"), config::options.esp.hide_dull_totems, false );

    setup_value( xorstr_("esp.dull_totem_color"), config::options.esp.dull_totem_color, config::options.esp.dull_totem_color );
    setup_value( xorstr_("esp.hex_totem_color"), config::options.esp.hex_totem_color, config::options.esp.hex_totem_color );
    setup_value( xorstr_("esp.hatch_closed_color"), config::options.esp.hatch_closed_color, config::options.esp.hatch_closed_color );
    setup_value( xorstr_("esp.hatch_open_color"), config::options.esp.hatch_open_color, config::options.esp.hatch_open_color );
    setup_value( xorstr_("esp.door_closed_color"), config::options.esp.door_closed_color, config::options.esp.door_closed_color );
    setup_value( xorstr_("esp.door_open_color"), config::options.esp.door_open_color, config::options.esp.door_open_color );
    setup_value( xorstr_("esp.pallet_up_color"), config::options.esp.pallet_up_color, config::options.esp.pallet_up_color );
    setup_value( xorstr_("esp.pallet_down_color"), config::options.esp.pallet_down_color, config::options.esp.pallet_down_color );
    setup_value( xorstr_("esp.hook_color"), config::options.esp.hook_color, config::options.esp.hook_color );
    setup_value( xorstr_("esp.chest_color"), config::options.esp.chest_color, config::options.esp.chest_color );
    setup_value( xorstr_("esp.locker_color"), config::options.esp.locker_color, config::options.esp.locker_color );
    setup_value( xorstr_("esp.trap_color"), config::options.esp.trap_color, config::options.esp.trap_color );

    setup_value( xorstr_("misc.autoskillcheck"), config::options.misc.autoskillcheck, false );
    setup_value( xorstr_("misc.autoskillcheck_key"), config::options.misc.autoskillcheck_key, VK_XBUTTON1 );
    setup_value( xorstr_("misc.autopallet"), config::options.misc.autopallet, false );
    setup_value( xorstr_("misc.autopallet_key"), config::options.misc.autopallet_key, VK_MENU );
}

void config::settings_manager::populate_scripts()
{
    for ( auto &file_iter : fs::directory_iterator { scripts_path } ) {
        if ( !file_iter.is_regular_file() ) {
            continue; // skip directories, symlinks, etc
        }

        fs::path file { file_iter };
        auto ext = file.extension();
        if ( !file.has_extension() || ( file.extension().compare( xorstr_( L".lua" ) ) != 0 ) ) {
            continue; // read only .lua files
        }

        std::ifstream stream { file };

        std::string content { std::istreambuf_iterator<char>( stream ), std::istreambuf_iterator<char>() };
        if ( content.empty() ) {
            continue;
        }

        options.scripts.script_data.emplace_back( file.filename().string(), content );
    }
}
