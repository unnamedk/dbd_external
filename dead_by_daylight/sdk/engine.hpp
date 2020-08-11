#pragma once

#include "../math/elem.hpp"
#include "base.hpp"
#include "methods.hpp"
namespace sdk
{
    using fvector2d = math::vector2;
    using fvector = math::vector3;
    using frotator = math::qangle;

    struct uactorcomponent : uobject
    {
        pad( 0xd0 );
    };

    struct uscenecomponent : uactorcomponent
    {
        pad( 0x7c );
        fvector relative_location;
        frotator relative_rotation;
        pad( 0xc + 0x30 );
        fvector velocity;
        pad( 0x64 );
        pad( 0x30 );
    };

    struct aactor : uobject
    {
        pad( 0xe0 );
        aactor *owner;
        pad( 0x40 );
        tarray<aactor *> children;
        uscenecomponent *root_component;

        pad( 0x1c0 );
        pad( 0x10 );
    };

    struct apawn;
    struct aplayerstate : aactor
    {
        float score;
        pad( 0x4 );
        fstring player_name;
        pad( 0x10 );
        int player_id;
        uint8_t ping;
        pad( 3 );
        int start_time;
        pad( 4 );
        pad( 20 );
        fstring saved_network_address;
        pad( 0x28 );
        pad( 0x10 );
        apawn *private_pawn;
        pad( 0x78 );
        fstring player_name_private;
        pad( 8 );
    };

    struct aplayercontroller;
    struct apawn : aactor
    {
        pad( 0x20 );
        aplayerstate *state;
        pad( 0x10 );
        aplayercontroller *controller;
        pad( 0x1c );
        int y; //pad( 0x4 );
    };

    struct fminimal_view_info
    {
        fvector location;
        frotator rotation;
        float fov;
        float desired_fov;
        float ortho_width;
        float ortho_near_clip_plane;
        float ortho_far_clip_plane;
        float aspect_ratio;
        std::uint8_t constrain_aspect_ratio : 1;
        std::uint8_t use_fov_for_lod : 1;
        pad( 0x3 );
        std::uint8_t projection_mode;
        pad( 0x3 );
        float post_process_blend_weight;
        pad( 0x4 );
        pad( 0x530 ); // FPostProcessSettings
        fvector2d off_center_projection_offset;
        pad( 0x8 );
    };

    struct fview_target
    {
        aactor *target;
        pad( 0x8 );
        fminimal_view_info pov;
        aplayerstate *player_state;
        pad( 0x8 );
    };

    struct fcamera_cache_entry
    {
        float timestamp;
        pad( 0xc );
        fminimal_view_info pov;
    };

    struct acamera_actor : aactor
    {
        pad( 0x24 );
        float aspect_ratio;
        float fov;
        float post_process_blend_weights;
        pad( 0x530 );
    };

    // wrong size probably
    struct aplayer_camera_manager : aactor
    {
        aplayercontroller *pc_owner;
        uscenecomponent *transform_component;
        pad( 0xc );
        float default_fov;
        pad( 0x4 );
        float default_ortho_width;
        pad( 0x4 );
        float default_aspect_ratio;
        pad( 0x40 );
        fcamera_cache_entry camera_cache;
        fcamera_cache_entry last_frame_camera_cache;
        fview_target view_target;
        fview_target pending_view_target;
        pad( 0xc28 );
        acamera_actor *camera_actor;
        pad( 0x30 );
    };

    /* struct aplayer_camera_manager : aactor
    {
        aplayercontroller *pc_owner;
        uscenecomponent *transform_component;
        pad( 0xc );
        float default_fov;
        pad( 0x4 );
        float default_ortho_width;
        pad( 0x4 );
        float default_aspect_ratio;
        pad( 0x40 );
        fcamera_cache_entry camera_cache;
        fcamera_cache_entry last_frame_camera_cache;
    };*/

    struct acontroller : aactor
    {
        pad( 0x8 );
        aplayerstate *player_state;
        pad( 0x28 );
        apawn *pawn;
        pad( 0x3b );
        pad( 0x3 );
    };

    struct uplayer : uobject
    {
        pad( 0x8 );
        aplayercontroller *player_controller;
        std::int32_t current_net_speed;
        std::int32_t configured_internet_speed;
        std::int32_t configured_lan_speed;
        std::int32_t unknown;
    };

    struct aplayercontroller : acontroller
    {
        uplayer *player;
        apawn *ack_pawn;
        pad( 0x10 );
        aplayer_camera_manager *camera_manager;
        pad( 0xc );
        math::qangle target_view_rotation;
        math::qangle unk_angle;
        float smooth_target_view_rot_speed;
    };

    struct ainfo : aactor
    {};

    struct uworld;
    struct ugameinstance;
    struct ugame_viewport_client : uobject
    {
        pad( 0x10 ); // UScriptViewportClient
        pad( 0x8 );
        void *viewport_console;
        pad( 0x10 );
        pad( 0x20 );
        uworld *world;
        ugameinstance *game_instance;
        pad( 0x268 );
    };

    struct ulocalplayer : uplayer
    {
        pad( 0x28 );
        ugame_viewport_client *viewport_client;
        pad( 0x10 );
        math::vector3 position;
    };

    struct ulevel : uobject
    {
        pad( 0xb0 - sizeof( uobject ) );
        tarray<aactor *> actor_list;
    };

    struct ugamestate : ainfo
    {
        uclass *game_mode_class;
        void *authority_game_mode;
        uclass *spectator_class;
        tarray<aplayerstate *> player_array;
        bool replicated_has_begun_play;
        pad( 0x3 );
        float replicated_world_time_seconds;
        float server_world_time_seconds_delta;
        float server_world_time_seconds_upd_freq;
        pad( 0x8 );
        pad( 0x28 );
    };

    struct ugameinstance : uobject
    {
        pad( 0x10 );
        tarray<ulocalplayer *> local_players;
        pad( 0x18 );
    };

    struct uworld : uobject
    {
        pad( 0x8 );
        ulevel *level;
        pad( 0xf0 );
        ugamestate *game_state;
        pad( 0x38 );
        ugameinstance *game_instance;
    };

    struct acharacter : apawn
    {
        pad( 0x3c0 );
    };
}