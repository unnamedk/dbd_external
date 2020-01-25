#pragma once

#include "base.hpp"
#include "engine.hpp"

namespace sdk
{
    struct foffering
    {
    };

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
    struct fcharacter_state_data
    {
        int pips;
        fname power_id;
        pad( 0x4 );
        tarray<fname> addon_ids;
    };
    struct fplayer_state_data
    {
        int character_level;
        fname equipped_favor_id;
        pad( 0x4 );
        tarray<fname> perk_ids;
        tarray<int> perk_levels;
        bool is_leaving_match;
        uint8_t player_game_state;
        pad( 2 );
        int prestige_level;
    };

    struct udbd_player_state : aplayerstate
    {
        fstring mirrors_id;
        fstring content_ver;
        bool is_player_ready;
        uint8_t game_role;
        pad( 0x1e );
        fcharacter_state_data camper_data;
        fcharacter_state_data slasher_data;
        fplayer_state_data player_data;
        pad( 0x250 + 0x20 );
        fstring platform_account_id;
        int selected_camper_index;
        int selected_slasher_index;
        pad( 0xe4 );
        uint8_t platform_flag;
        pad( 0x3 );
        bool crossplay_allowed;
        bool game_level_loaded;
        pad( 0x6 );
    };
    struct ubase_game_modifier_container : uactorcomponent
    {
        pad( 0x8 );
        fname id;
        pad( 0x10 );
    };
    struct uperk : ubase_game_modifier_container
    {
        pad( 0xd0 );
        pad( 0x48 );
        pad( 0x30 * 3 );
        int perk_level;
        pad( 0x34 );
        pad( 0x8 );
        pad( 0x10 );
        bool is_usable;
        bool BroadcastWhenApplicable; // 0x0319(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
        bool BroadcastOnTimer; // 0x031A(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
        bool BroadcastCooldownTimer; // 0x031B(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
        bool BroadcastInactiveCooldownTimer; // 0x031C(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
        bool BroadcastAlways;  
    };
    struct uperk_collection_component : uactorcomponent
    {
        tarray<uperk *> _array;
        pad( 0x18 );
    };

    struct uperk_manager : uactorcomponent
    {
        pad( 0x10 );
        uperk_collection_component *perks;
        std::uintptr_t status_effects;
        pad( 0x1e8 );
    };

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
        // BaseDBDPlayer
        char ipad[ 0x10 ];
        int character_index;
        pad( 0x1c );

        // DBDPlayer
        pad( 0x3d0 );
        tarray<foffering> offerings;
        pad( 0x8 );
        uplayer_interaction_handler *interaction_handler;
        pad( 0x8 );
        uperk_manager *perk_manager;
        udbd_player_data *player_data;
        pad( 0x768 );
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
        pad( 0xe8 );
        bool is_in_terror_radius;
        bool trap_indicator_active;
    };
    struct aslasher_player : adbdplayer
    {
        pad( 0x308 );
        bool is_killing;
        int8_t allowed_kill_count;
        int8_t allowed_kill_after_hooking;
        bool allowed_kill_last_survivor;
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
    struct reverse_bear_trap_remover_t : sdk::ainteractable
    {
        tarray<int> keys;
        pad( 0x50 );
        uintptr_t map_actor;
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