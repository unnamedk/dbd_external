#pragma once

#include <string_view>

#include <windows.h>

#include <d3d11.h>

#include <string>
#include <unordered_map>

namespace overlay
{
    class menu
    {
    public:
        menu() noexcept
            : m_window_title()
            , m_window( nullptr )
            , m_device( nullptr )
            , m_device_context( nullptr )
            , m_target_view( nullptr )
            , m_swap_chain( nullptr )
        {}

        bool initialize( std::wstring_view window_title );
        void poll();

        const std::wstring &title() const;
        void title( std::wstring_view new_title );

        static LRESULT WINAPI window_proc( HWND window, std::uint32_t msg, WPARAM wp, LPARAM lp );

        const std::unordered_map<std::string, ID3D11ShaderResourceView *> &textures() const noexcept { return this->m_textures; }
        std::unordered_map<std::string, ID3D11ShaderResourceView *> &textures() noexcept { return this->m_textures; }

        static menu &get()
        {
            static menu ans;
            return ans;
        }

    private:
        bool load_textures();
        bool init_d3d_devices();

        void set_style();
        void create_render_target();
        void cleanup_render_target();

        void draw_menu();
        void draw_colors_tab();
        void draw_esp_tab();
        void draw_misc_tab();
        void draw_scripts_tab();
        void draw_player_list();
        void draw_changelog_tab();

        void setup_frame();
        void end_frame();

        std::wstring m_window_title;

        HWND m_window;

        std::unordered_map<std::string, ID3D11ShaderResourceView *> m_textures;

        ID3D11Device *m_device;
        ID3D11DeviceContext *m_device_context;
        ID3D11RenderTargetView *m_target_view;
        IDXGISwapChain *m_swap_chain;
    };
}