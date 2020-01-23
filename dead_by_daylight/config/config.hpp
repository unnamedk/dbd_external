#pragma once

#include "../sdk/color.hpp"
#include "parser.hpp"
#include "../cheats/scripts.hpp"

#include <filesystem>
#include <string>
#include <optional>

namespace config
{
    struct settings_manager
    {
        template <typename T>
        void setup_value( std::string &&path, T &variable, T default_value )
        {
            static_assert( false );
        }

        template <>
        void setup_value<bool>( std::string &&path, bool &variable, bool default_value )
        {
            bools.emplace_back( std::forward<std::string>( path ), variable, default_value );
        }
        template <>
        void setup_value<float>( std::string &&path, float &variable, float default_value )
        {
            floats.emplace_back( std::forward<std::string>( path ), variable, default_value );
        }
        template <>
        void setup_value<int>( std::string &&path, int &variable, int default_value )
        {
            ints.emplace_back( std::forward<std::string>( path ), variable, default_value );
        }
        template <>
        void setup_value<std::string>( std::string &&path, std::string &variable, std::string default_value )
        {
            strings.emplace_back( std::forward<std::string>( path ), variable, default_value );
        }
        template <>
        void setup_value<sdk::color_t>( std::string &&path, sdk::color_t &variable, sdk::color_t default_value )
        {
            colors.emplace_back( std::forward<std::string>( path ), variable, default_value );
        }

        template <typename T>
        T get_value( const T &default_value, const std::string &path )
        {
            return main_cfg.get_or_insert<T>( default_value, path );
        }

        void init();

        void load();
        void save();

    private:
        template <typename T>
        struct value_t
        {
            value_t( std::string &&s, T &val, T def_val )
                : path( std::move( s ) )
                , variable( val )
                , default_value( def_val )
            {}

            std::string path;
            T &variable;
            T default_value;
        };

        std::vector<value_t<int>> ints;
        std::vector<value_t<bool>> bools;
        std::vector<value_t<float>> floats;
        std::vector<value_t<std::string>> strings;
        std::vector<value_t<sdk::color_t>> colors;

        void populate_settings();
        void populate_scripts();

        std::filesystem::path configs_path;
        std::filesystem::path scripts_path;
        
        parser_t main_cfg;
    };

    extern std::optional<settings_manager> config_system;
}