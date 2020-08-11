#pragma once

#include <array>
#include <cstdint>

#include "../sdk/color.hpp"
#include "../cheats/scripts.hpp"

namespace config
{
    enum esp_filter_t : std::int32_t
    {
        killers = ( 1 << 0 ),
        survivors = ( 1 << 1 ),
        totems = ( 1 << 2 ),
        hatches = ( 1 << 3 ),
        generators = ( 1 << 4 ),
        doors = ( 1 << 5 ),
        pallets = ( 1 << 6 ),
        chests = ( 1 << 7 ),
        items = ( 1 << 8 ),
        hooks = ( 1 << 9 ),
        lockers = ( 1 << 10 ),
        breakable_wall = ( 1 << 11 )
    };
    extern const std::array<const char *, 124> keys_list;

    struct options_t
    {
        struct
        {
            int key = 0x10;
            int aim_pred_key = 0x10;
            float fov = 40.f;
        } aim;

        struct
        {
            bool enabled = false;
            bool debug_mode = false;

            std::int32_t filter_flags = ( killers | survivors | totems | hatches );

            std::array<std::int32_t, 13> priority_table {
                0, // unknown
                9, // survivor
                10, // killer
                6, // item
                5, // killer traps
                8, // gens
                7, // hatch
                7, // doors
                6, // totems
                1, // lockers
                2, // hooks
                3, // pallets
                4, // chests
            };

            bool radar = false;
            bool radar_preview_icons = false;
            bool hide_dull_totems = false;
            float radar_icon_scale = 4.5f;
            float radar_zoom = 16.f;
            float radar_image_size = 20.f;

            sdk::color_t dull_totem_color = sdk::color_t( 0xE4552BFFu );
            sdk::color_t hex_totem_color = sdk::color_t( 0x47EF7CFFu );

            sdk::color_t hatch_closed_color = sdk::color_t( 0xFFF500FFu );
            sdk::color_t hatch_open_color = sdk::color_t( 0x00FFE1FFu );

            sdk::color_t door_closed_color = sdk::color_t( 0xFFF500FFu );
            sdk::color_t door_open_color = sdk::color_t( 0x00FFE1FFu );

            sdk::color_t pallet_up_color = sdk::color_t( 0x00aeffffu );
            sdk::color_t pallet_down_color = sdk::color_t::purple();

            sdk::color_t hook_color = sdk::color_t::white();
            sdk::color_t chest_color = sdk::color_t::white();
            sdk::color_t locker_color = sdk::color_t::white();
            sdk::color_t trap_color = sdk::color_t::white();
        } esp;

        struct
        {
            bool master_switch = true;
            bool should_run_threads = true;

            bool autoskillcheck = true;
            int autoskillcheck_key = 0x5;

            bool autopallet = true;
            int autopallet_key = 0x5;

            bool auto_borrowed = false;
            int auto_borrowed_key = 0x5;

            bool auto_headon = false;
            int auto_headon_key = 0x5;

            bool turn_keys = false;
            float turn_speed = 0.5f;
            int turn_key_left = 0;
            int turn_key_right = 0;

            float gravity = 9.81f;
            float fov = 30.f;
        } misc;

        struct
        {
            std::vector<cheats::basic_script> script_data;
        } scripts;
    };

    extern options_t options;
}