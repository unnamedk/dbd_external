#include "menu.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_custom.hpp"
#include "imgui/dx11/imgui_impl_win32.h"
#include "imgui/dx11/imgui_impl_dx11.h"

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

#include <DirectXTK/WICTextureLoader.h>

#include <filesystem>
#include <xorstr.hpp>

#include "../config/options.hpp"
#include "../cheats/esp.hpp"
#include "../config/config.hpp"
#include "../cheats/actor_manager.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

static ImVec4 clear_color = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );

bool overlay::menu::initialize( std::wstring_view window_title )
{
    this->m_window_title = window_title;

    // register and create window
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, window_proc, 0L, 0L, GetModuleHandle( nullptr ), nullptr, nullptr, nullptr, nullptr, std::data( this->m_window_title ), nullptr };
    RegisterClassEx( &wc );

    this->m_window = CreateWindow( wc.lpszClassName, std::data( this->m_window_title ), WS_OVERLAPPEDWINDOW, 100, 100, 640, 480, nullptr, nullptr, nullptr, nullptr );
    if ( !this->m_window ) {
        return false;
    }

    // create d3d device
    if ( !this->init_d3d_devices() ) {
        return false;
    }

    ShowWindow( this->m_window, SW_SHOWDEFAULT );
    ShowWindow( GetConsoleWindow(), SW_HIDE );
    UpdateWindow( this->m_window );

    ImGui::CreateContext();
    this->set_style();
    return ImGui_ImplWin32_Init( this->m_window ) && ImGui_ImplDX11_Init( this->m_device, this->m_device_context ) && this->load_textures();
}

void overlay::menu::poll()
{
    MSG msg {};
    while ( ( msg.message != WM_QUIT ) && ( config::options.misc.should_run_threads ) ) {
        if ( PeekMessage( &msg, nullptr, 0u, 0u, PM_REMOVE ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
            continue;
        }

        this->setup_frame();

        this->draw_menu();

        this->end_frame();
    }

    config::config_system->save();
}

const std::wstring &overlay::menu::title() const
{
    return this->m_window_title;
}

void overlay::menu::title( std::wstring_view new_title )
{
    SetWindowText( this->m_window, std::data( new_title ) );
}

LRESULT __stdcall overlay::menu::window_proc( HWND window, std::uint32_t msg, WPARAM wp, LPARAM lp )
{
    if ( ImGui_ImplWin32_WndProcHandler( window, msg, wp, lp ) ) {
        return true;
    }

    switch ( msg ) {
        case WM_SIZE: {
            menu::get().cleanup_render_target();
            menu::get().m_swap_chain->ResizeBuffers( 0u, LOWORD( lp ), HIWORD( lp ), DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, 0u );
            menu::get().create_render_target();
            return 0;
        }

        case WM_SYSCOMMAND: {
            if ( ( wp & 0xfff0 ) == SC_KEYMENU ) {
                return 0;
            }
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage( 0 );
            return 0;
        }
    }

    return DefWindowProc( window, msg, wp, lp );
}

bool overlay::menu::load_textures()
{
    const auto icons_folder = std::filesystem::current_path() / "icons";
    if ( !std::filesystem::exists( icons_folder ) ) {
        return false;
    }

    for ( auto &f : std::filesystem::directory_iterator { icons_folder } ) {
        if ( !f.is_regular_file() ) {
            continue;
        }

        auto file = f.path();
        if ( !file.has_stem() ) {
            continue;
        }

        DirectX::CreateWICTextureFromFile( this->m_device, file.c_str(), nullptr, &this->m_textures[ file.stem().string() ] );
    }

    return true;
}

bool overlay::menu::init_d3d_devices()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = this->m_window;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    auto flags = 0u;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[ 2 ] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    if ( D3D11CreateDeviceAndSwapChain( NULL,
             D3D_DRIVER_TYPE_HARDWARE,
             NULL,
             flags,
             featureLevelArray,
             2, D3D11_SDK_VERSION,
             &sd,
             &this->m_swap_chain,
             &this->m_device,
             &featureLevel,
             &this->m_device_context ) != S_OK ) {
        return false;
    }

    this->create_render_target();
    return true;
}

void overlay::menu::set_style()
{
    auto &style = ImGui::GetStyle();

    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;
    style.CurveTessellationTol = 1.25f;
    style.WindowPadding = { 7.5, 5.f };
    style.WindowRounding = 12.f;
    style.ChildRounding = 12.f;
    style.GrabRounding = 16.f;
    style.TabRounding = 4.f;
    style.FramePadding = { 4.f, 3.f };
    style.FrameRounding = 5.f;
    style.ItemSpacing = { 8.f, 4.f };
    style.ItemInnerSpacing = { 4.f, 4.f };
    style.TouchExtraPadding = { 0.f, 0.f };
    style.IndentSpacing = 21.f;
    style.ScrollbarSize = 11.f;
    style.ScrollbarRounding = 16.f;
    style.GrabMinSize = 11.f;
    style.GrabRounding = 16.f;
    style.WindowTitleAlign = { 0.5f, 0.5f };
    style.ButtonTextAlign = { 0.5f, 0.5f };
    style.Alpha = 0.93f;

    ImVec4 *colors = style.Colors;
    colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
    colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
    colors[ ImGuiCol_WindowBg ] = ImVec4( 0.06f, 0.06f, 0.06f, 0.94f );
    colors[ ImGuiCol_Border ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.19f );
    colors[ ImGuiCol_PopupBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.94f );
    colors[ ImGuiCol_FrameBg ] = ImVec4( 0.16f, 0.29f, 0.48f, 0.54f );
    colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.40f );
    colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
    colors[ ImGuiCol_TitleBg ] = ImVec4( 0.18f, 0.18f, 0.18f, 1.00f );
    colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
    colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.00f );
    colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
    colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
    colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.78f, 1.00f );
    colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.78f, 1.00f );
    colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 1.f, 1.00f );
    colors[ ImGuiCol_CheckMark ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.24f, 0.52f, 0.88f, 1.00f );
    colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ ImGuiCol_Button ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.40f );
    colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.06f, 0.53f, 0.98f, 1.00f );
    colors[ ImGuiCol_Header ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
    colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
    colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ ImGuiCol_Separator ] = colors[ ImGuiCol_Border ];
    colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.78f );
    colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.25f );
    colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
    colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
    colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
    colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
    colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
    colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
    colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
    colors[ ImGuiCol_ModalWindowDarkening ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
}

void overlay::menu::create_render_target()
{
    ID3D11Texture2D *bb = nullptr;
    this->m_swap_chain->GetBuffer( 0, IID_PPV_ARGS( &bb ) );
    this->m_device->CreateRenderTargetView( bb, nullptr, &this->m_target_view );
    bb->Release();
}

void overlay::menu::cleanup_render_target()
{
    if ( this->m_target_view ) {
        this->m_target_view->Release();
        this->m_target_view = nullptr;
    }
}

void overlay::menu::draw_menu()
{
    ImGui::SetNextWindowSize( ImVec2( 480, 500 ), ImGuiCond_FirstUseEver );
    if ( !ImGui::Begin( "keylogger manager v3", nullptr, ImGuiWindowFlags_NoCollapse ) ) {
        ImGui::End();
        return;
    }

    if ( ImGui::BeginMainMenuBar() ) {
        if ( ImGui::BeginMenu( xorstr_( "settings" ) ) ) {
            if ( ImGui::MenuItem( ( xorstr_( "save config" ) ) ) ) {
                config::config_system->save();
            }

            if ( ImGui::MenuItem( xorstr_( "load config" ) ) ) {
                config::config_system->load();
            }

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( xorstr_( "cheat" ) ) ) {
            if ( ImGui::MenuItem( ( xorstr_( "re-cache gnames" ) ) ) ) {
                cheats::actor_manager->update_names();
            }

            if ( ImGui::MenuItem( xorstr_( "reload textures" ) ) ) {
                this->m_textures.clear();
                this->load_textures();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if ( ImGui::BeginTabBar( "assistances", ImGuiTabBarFlags_Reorderable ) ) {
        if ( ImGui::BeginTabItem( "esp" ) ) {
            draw_esp_tab();
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "misc" ) ) {
            draw_misc_tab();
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "colors" ) ) {
            draw_colors_tab();
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "changelog" ) ) {
            draw_changelog_tab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void overlay::menu::draw_colors_tab()
{
    ImGui::BeginChild( "colors" );
    constexpr auto flags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar;

    ImGui::ColorEdit4( "dull totem", config::options.esp.dull_totem_color.data(), flags );
    ImGui::ColorEdit4( "hex totem", config::options.esp.hex_totem_color.data(), flags );
    ImGui::ColorEdit4( "closed hatch", config::options.esp.hatch_closed_color.data(), flags );
    ImGui::ColorEdit4( "open hatch", config::options.esp.hatch_open_color.data(), flags );
    ImGui::ColorEdit4( "closed door", config::options.esp.door_closed_color.data(), flags );
    ImGui::ColorEdit4( "open door", config::options.esp.door_open_color.data(), flags );
    ImGui::ColorEdit4( "up pallet", config::options.esp.pallet_up_color.data(), flags );
    ImGui::ColorEdit4( "downed pallet", config::options.esp.pallet_down_color.data(), flags );
    ImGui::ColorEdit4( "hook", config::options.esp.hook_color.data(), flags );
    ImGui::ColorEdit4( "chest", config::options.esp.chest_color.data(), flags );
    ImGui::ColorEdit4( "locker", config::options.esp.locker_color.data(), flags );
    ImGui::ColorEdit4( "trap", config::options.esp.trap_color.data(), flags );

    ImGui::BeginChild( "icons_preview", {}, true );
    auto preview_icon = [this]( const std::string &name, sdk::color_t clr ) {
        ImGui::Image( this->m_textures[ name ], ImVec2( 50, 50 ), { 0, 0 }, { 1, 1 }, ImVec4( clr.clamped_r(), clr.clamped_g(), clr.clamped_b(), clr.clamped_a() ), {} );
    };

    {
        preview_icon( "totem", config::options.esp.dull_totem_color );
        ImGui::SameLine();
        preview_icon( "noed", config::options.esp.hex_totem_color );
        ImGui::SameLine();
        preview_icon( "ruin", config::options.esp.hex_totem_color );
        ImGui::SameLine();
        preview_icon( "third_seal", config::options.esp.hex_totem_color );
        ImGui::SameLine();
        preview_icon( "thrill", config::options.esp.hex_totem_color );
        ImGui::SameLine();
        preview_icon( "hauntedground", config::options.esp.hex_totem_color );
        ImGui::SameLine();
        preview_icon( "huntresslullaby", config::options.esp.hex_totem_color );
    }

    {
        preview_icon( "hatch", config::options.esp.hatch_closed_color );
        ImGui::SameLine();

        preview_icon( "hatch", config::options.esp.hatch_open_color );
        ImGui::SameLine();

        preview_icon( "door", config::options.esp.door_closed_color );
        ImGui::SameLine();

        preview_icon( "door", config::options.esp.door_open_color );
    }

    {
        preview_icon( "pallet", config::options.esp.pallet_up_color );
        ImGui::SameLine();

        preview_icon( "pallet", config::options.esp.pallet_down_color );
    }

    {
        preview_icon( "hillbilly", sdk::color_t::white() );
        ImGui::SameLine();
        preview_icon( "freddy", sdk::color_t::white() );
        ImGui::SameLine();
        preview_icon( "doctor", sdk::color_t::white() );
        ImGui::SameLine();
        preview_icon( "bubba", sdk::color_t::white() );
    }

    {
        preview_icon( "meg_thomas", sdk::color_t::white() );
        ImGui::SameLine();
        preview_icon( "dwight_fairfield", sdk::color_t::white() );
        ImGui::SameLine();
        preview_icon( "nea_karlsson", sdk::color_t::white() );
        ImGui::SameLine();
        preview_icon( "yui_kimura", sdk::color_t::white() );
    }

    {
        preview_icon( "hook", config::options.esp.hook_color );
        ImGui::SameLine();

        preview_icon( "chest", config::options.esp.chest_color );
        ImGui::SameLine();

        preview_icon( "locker", config::options.esp.locker_color );
        ImGui::SameLine();

        preview_icon( "trap", config::options.esp.trap_color );
    }

    ImGui::EndChild();

    ImGui::EndChild();
}

void overlay::menu::draw_esp_tab()
{
    ImGui::BeginChild( "esp_tab" );
    ImGui::Text( "esp" );
    ImGui::Separator();

    ImGui::Checkbox( "enabled", &config::options.esp.enabled );
    ImGui::Checkbox( "debug mode", &config::options.esp.debug_mode );
    ImGui::Separator();

    ImGui::SelectableFlags( "killer", &config::options.esp.filter_flags, config::killers );
    ImGui::SelectableFlags( "survivor", &config::options.esp.filter_flags, config::survivors );
    ImGui::SelectableFlags( "totem", &config::options.esp.filter_flags, config::totems );
    ImGui::SelectableFlags( "hatch", &config::options.esp.filter_flags, config::hatches );
    ImGui::SelectableFlags( "pallets", &config::options.esp.filter_flags, config::pallets );

    ImGui::SelectableFlags( "doors", &config::options.esp.filter_flags, config::doors );
    ImGui::SelectableFlags( "generators", &config::options.esp.filter_flags, config::generators );
    ImGui::SelectableFlags( "chests", &config::options.esp.filter_flags, config::chests );
    ImGui::SelectableFlags( "items", &config::options.esp.filter_flags, config::items );
    ImGui::SelectableFlags( "hooks", &config::options.esp.filter_flags, config::hooks );
    ImGui::SelectableFlags( "lockers", &config::options.esp.filter_flags, config::lockers );

    ImGui::Separator();
    ImGui::Checkbox( "hide dull totems", &config::options.esp.hide_dull_totems );
    ImGui::Checkbox( "render icons", &config::options.esp.radar_preview_icons );
    ImGui::SliderFloat( "icon scale", &config::options.esp.radar_icon_scale, 3.f, 10.f );
    ImGui::SliderFloat( "image size", &config::options.esp.radar_image_size, 5.f, 100.f );
    ImGui::SliderFloat( "radar zoom", &config::options.esp.radar_zoom, 2.f, 50.f );

    if ( ImGui::CollapsingHeader( "priorities" ) ) {
        ImGui::BeginChild( "priorities_child", {}, true );
        auto make_priority_button = []( const char *name, int &priority ) {
            ImGui::Text( "%s", name );
            ImGui::NextColumn();

            // lol
            char id[ 5 ] = { 0 };
            std::copy( name, name + 3, id );
            id[ 2 ] = 'y';

            if ( ImGui::ArrowButton( id, ImGuiDir_Left ) ) {
                priority = std::max( priority - 1, 0 );
            }

            ImGui::SameLine();
            ImGui::Text( "%i", priority );

            id[ 2 ] = 'x';

            ImGui::SameLine();
            if ( ImGui::ArrowButton( id, ImGuiDir_Right ) ) {
                priority = std::min( 10, priority + 1 );
            }
            ImGui::NextColumn();
        };

        ImGui::Columns( 2 );
        ImGui::Separator();
        ImGui::Text( "type" );
        ImGui::NextColumn();
        ImGui::Text( "priority" );
        ImGui::NextColumn();

        make_priority_button( "survivors", config::options.esp.priority_table[ 1 ] );
        make_priority_button( "killers", config::options.esp.priority_table[ 2 ] );
        make_priority_button( "items", config::options.esp.priority_table[ 3 ] );
        make_priority_button( "killer traps", config::options.esp.priority_table[ 4 ] );
        make_priority_button( "generators", config::options.esp.priority_table[ 5 ] );
        make_priority_button( "hatches", config::options.esp.priority_table[ 6 ] );
        make_priority_button( "doors", config::options.esp.priority_table[ 7 ] );
        make_priority_button( "totems", config::options.esp.priority_table[ 8 ] );
        make_priority_button( "lockers", config::options.esp.priority_table[ 9 ] );
        make_priority_button( "hooks", config::options.esp.priority_table[ 10 ] );
        make_priority_button( "pallets", config::options.esp.priority_table[ 11 ] );
        make_priority_button( "chests", config::options.esp.priority_table[ 12 ] );
        make_priority_button( "unknown", config::options.esp.priority_table[ 0 ] );

        ImGui::EndChild();
    }

    if ( cheats::esp ) {
        cheats::esp->run();
    }

    ImGui::EndChild();
}

void overlay::menu::draw_misc_tab()
{
    ImGui::BeginChild( "misc_tab", {}, true );
    ImGui::Text( "misc" );
    ImGui::Separator();

    ImGui::Checkbox( "auto skillcheck", &config::options.misc.autoskillcheck );
    if ( config::options.misc.autoskillcheck ) {
        ImGui::Combo( "auto skillcheck key", &config::options.misc.autoskillcheck_key, config::keys_list.data(), config::keys_list.size() );
    }
    ImGui::Checkbox( "auto pallet", &config::options.misc.autopallet );
    if ( config::options.misc.autopallet ) {
        ImGui::Combo( "auto pallet key", &config::options.misc.autopallet_key, config::keys_list.data(), config::keys_list.size() );
    }

    ImGui::EndChild();
}

void overlay::menu::draw_changelog_tab()
{
    ImGui::BeginChild( "changelog_tab", {}, true );

    static std::array changelist {
        std::make_pair( "1.0.0", "initial release" ),
    };

    ImGui::Columns( 2 );
    ImGui::Separator();
    ImGui::Text( "version" );
    ImGui::NextColumn();
    ImGui::Text( "changes" );
    ImGui::NextColumn();

    for ( auto &[ v, t ] : changelist ) {
        ImGui::Text( "%s\n", v );
        ImGui::NextColumn();

        ImGui::Text( "%s\n", t );
        ImGui::NextColumn();
    }

    ImGui::EndChild();
}

void overlay::menu::setup_frame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void overlay::menu::end_frame()
{
    ImGui::Render();
    this->m_device_context->OMSetRenderTargets( 1, &this->m_target_view, NULL );
    this->m_device_context->ClearRenderTargetView( this->m_target_view, ( float * ) &clear_color );
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

    this->m_swap_chain->Present( 1, 0 ); // Present with vsync
}