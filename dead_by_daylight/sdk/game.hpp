#pragma once

#include "base.hpp"
#include "engine.hpp"

namespace sdk
{
    struct foffering
    {};

    struct uplayer_interaction_handler
    {
        struct uskillcheck
        {
            pad( 0x270 );
            float success_zone_start;
            float success_zone_end;
            float bonus_zone_length;
            int skillcheck_count;
            int x_offset;
            float x_offset_max;
            int y_offset;
            float y_offset_max;
            float insane_skillcheck_jitter_amp;
            pad( 0xc );
            float current_progress_rate;
        };

        pad( 0x278 );
        uskillcheck *skillcheck;
    };

    struct uperk_manager
    {};

    struct udbd_player_data
    {};

    struct ustillness_tracker
    {
        pad( 0x100 );
        float speed_threshold;
        float distance_threshold;
        float distance_decay;
        float decay_while_moving;
        pad( 0x3 );
        float stillness_threshold;
        float stillness_timer_limit;
        float decay_speed_multiplier;
        float stillness_timer;
        float unknown;
    };

    struct adbdplayer : acharacter
    {
        char ipad[ 0x10 ];
        int character_index;
        pad( 0x1c );
        pad( 0x3f0 );
        tarray<foffering> offerings;
        pad( 0x8 );
        uplayer_interaction_handler *interaction_handler;
        pad( 0x8 );
        uperk_manager *perk_manager;
        udbd_player_data *player_data;
        pad( 0x728 );
    };
    struct ucamper_health_component : uactorcomponent
    {
        pad( 0xd4 );
        int current_health_state_count;
        pad( 0x4 );
        int num_healthy_heals;
        // ...
    };
    struct acamper_player : adbdplayer
    {
        pad( 0x130 );
        ustillness_tracker *stillness_tracker;
        pad( 0xe0 );
        ucamper_health_component *health_component;
    };
    struct aslasher_player : adbdplayer
    {
        pad( 0x308 );
        bool is_killing;
    };
    struct ainteractable : aactor
    {
        pad( 0x88 );
    };
    struct ameathook : ainteractable
    {
        pad( 0x8 );
        bool is_survivor_struggling;
        bool was_survivor_struggle_canceled;
        pad( 0x6 + 0x10 );
        bool is_in_basement;
        bool is_breakable;
        bool is_sabotageable;
        pad( 0xaf );
        bool is_sacrificed;
        bool is_sabotaged;
        bool survivor_unhookable;
        bool can_survivor_attempt_escape;
        bool survivor_can_struggle;
        bool can_be_sabotaged;
    };
    struct agenerator : ainteractable
    {
        bool activated;
        bool is_repaired;
        bool force_reveal_to_local_player;
        pad( 0xd );
        float native_percent_complete;
        pad( 0xa0 );
        bool is_blocked;
        bool is_blocked_from_charging;
    };
    struct aescapedoor : ainteractable
    {
        bool activated;
    };
    struct alocker : ainteractable
    {
        enum class closet_anim_state : std::uint8_t
        {
            none,
            open = 3,
            open_fast = 6,
            search_empty = 9
        };

        pad( 0x10 );
        sdk::adbdplayer *delayed_door_open_player;
        closet_anim_state delayed_door_open_state;
        pad( 0x7 );
        sdk::acamper_player *survivor_in_locker;
        pad( 0x8 + 0x18 + 0x20 );
    };
    struct ahatch : ainteractable
    {
        enum class hatch_state_t : std::uint8_t
        {
            default_close,
            default_open,
            forced_close,
            key_open // have to test
        };

        hatch_state_t hatch_state;
        bool is_survivor_escaping;
        bool is_active;
        pad( 0x5 );
    };
    struct apallet : ainteractable
    {
        enum class pallet_state_t : std::uint8_t
        {
            up,
            broken = 3,
            illusionary = 4
        };

        bool stun_sent;
        pallet_state_t pallet_state;
        pad( 0x56 );
        void *stun_box;
    };
    struct atotem : ainteractable
    {
        fname hex_perk_id;
        pad( 0x4 );
        bool is_cleansed;
        pad( 0x13 );
    };
    struct aitem : ainteractable
    {
        // depends on the item
    };
    struct asearchable : ainteractable
    {
        float weight;
        std::uint8_t rarity;
        pad( 0x3 );
        pad( 0x8 );
        fname item_id_to_spawn;
        pad( 0x4 );
        bool has_been_searched;
        pad( 0x13 );
    };

    struct udbd_game_state : ugamestate
    {
        struct built_level_data_t
        {
            fname theme_name;
            pad( 0x4 );
            fname theme_weather;
            pad( 0x4 );
            fstring map_name;

            pad( 0x28 );
        };

        struct offering_data_t
        {
            struct fselected_offering
            {
                int owner_id;
                fname offering_name;
                pad( 0x4 );
            };

            bool offering_ready;
            pad( 0x7 );
            tarray<fselected_offering> offerings;
        };

        pad( 0x150 );

        std::int32_t camper_dead_count;
        std::int32_t camper_hooked_count;
        std::int32_t camper_escaped;
        std::int32_t waiting_for_escape;
        std::int32_t camper_count;

        pad( 0x4 );

        std::int32_t seconds_left_in_lobby;

        pad( 0x34 );

        aslasher_player *slasher;
        pad( 0x10 );
        std::int32_t camper_hooked_in_basement_count;
        pad( 0xc );

        built_level_data_t built_level_data;
        offering_data_t offerings;
        pad( 0xa8 );
        std::int32_t activated_gen_count;
        std::int32_t required_activated_gen_count;
        bool escape_door_activated;
        bool escape_door_opened;
        bool hatch_open;
        bool level_ready_to_play;
        bool player_distribution_ready;
        bool using_weakened_mechanic;
        pad( 0x2 );
        std::int32_t survivor_left;
        pad( 0x4 );
        tarray<ameathook *> meathooks;
        tarray<agenerator *> generators;
        tarray<aescapedoor *> escape_doors;
        tarray<ahatch *> hatches;
        tarray<apallet *> pallets;
        tarray<atotem *> totems;
        pad( 0x50 );
        tarray<ainteractable *> traps;
    };

    struct udbd_game_instance : ugameinstance
    {
    };
}