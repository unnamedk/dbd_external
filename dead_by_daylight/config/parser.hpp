#pragma once

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>

// Add std::optional serializer
namespace nlohmann
{
    template <typename T>
    struct adl_serializer<std::optional<T>>
    {
        static void to_json( json &j, const std::optional<T> &opt )
        {
            if ( opt.has_value() ) {
                j = nullptr;
            } else {
                j = *opt;
            }
        }

        static void from_json( const json &j, std::optional<T> &opt )
        {
            if ( !j.is_null() ) {
                opt = j.get<T>();
            } else {
                opt = {};
            }
        }
    };
}

namespace config
{
    namespace fs = std::filesystem;
	using nlohmann::json;

    class parser_t
    {
    public:
        /*
        * If there's no value on the wanted path, writes the return value
        * into the path and returns it
        */
        template <typename T>
        T get_or_insert( const T &default_value, const std::string &path )
        {
            json &object = get_impl( split( path, "." ) );
            if ( object.empty() ) {
                object = default_value;
                return default_value;
            }

            return object.get<T>();
        }

        template <typename T, typename... Args>
        void put( const T &value, const std::string &path )
        {
            // Get the object from the wanted path
            json &object = get_impl( split( path, "." ) );

            // Assign value to node
            object = value;
        }

        void save_to_disk()
        {
            this->m_ostr.close();
            fs::remove( this->m_path_to_file );

            m_ostr.close();
            m_ostr = std::ofstream( m_path_to_file );
            m_ostr << std::setw( 4 ) << m_data << std::endl;
        }

        void set_config_file( const fs::path &path_to_file )
        {
            prepare_file( path_to_file );
        }

        fs::path &get_path()
        {
            return m_path_to_file;
        }

        friend std::ostream &operator<<( std::ostream &ostr, const parser_t &obj )
        {
            return ostr << obj.m_data;
        }

        parser_t() {}
        parser_t( const fs::path &path_to_file ) { prepare_file( path_to_file ); }

        ~parser_t() { this->save_to_disk(); }

    private:
        void prepare_file( const fs::path &path )
        {
            this->m_path_to_file = path;

            // If the file doesn't exist, create it
            if ( !fs::exists( m_path_to_file ) ) {
                this->m_ostr = std::ofstream( m_path_to_file );
            }

            // If it does, and isn't empty, parse its contents
            else {
                auto istr = std::ifstream( m_path_to_file );
                if ( istr.peek() != std::fstream::traits_type::eof() ) {
                    istr >> m_data;
                }
            }
        }

        std::vector<std::string> split( const std::string &str, const std::string &delim )
        {
            std::vector<std::string> tokens;
            size_t prev = 0, pos = 0;
            do {
                pos = str.find( delim, prev );
                if ( pos == std::string::npos ) {
                    pos = str.length();
                }

                std::string token = str.substr( prev, pos - prev );

                if ( !token.empty() ) {
                    tokens.push_back( token );
                }

                prev = pos + delim.length();
            } while ( pos < str.length() && prev < str.length() );

            return tokens;
        }

        json &get_impl( const std::vector<std::string> &path )
        {
            json *node_ptr = &m_data;

            for ( auto &elem : path ) {
                auto &node_obj = *node_ptr;
                if ( !node_ptr->is_object() && !node_ptr->is_null() ) {
                    node_ptr->erase( node_ptr->begin(), node_ptr->end() );
                }

                node_ptr = &( node_obj[ elem ] );
            }

            return *node_ptr;
        }

        fs::path m_path_to_file;
        std::ofstream m_ostr;
        json m_data;
    };
}