#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#include <filesystem>

#include "overlay/menu.hpp"
#include <xorstr.hpp>
#include <native/process.hpp>

#include "cheats/offsets.hpp"
#include "cheats/esp.hpp"
#include "cheats/utilities.hpp"
#include "cheats/actor_manager.hpp"
#include "config/config.hpp"

namespace fs = std::filesystem;
using namespace std::chrono_literals;

#define RETURN_ERROR( fmt, ... )       \
    spdlog::error( fmt, __VA_ARGS__ ); \
    ( void ) getchar();                \
    return 1;

nt::process_kernel wait_process( std::vector<std::wstring_view> name )
{
    while ( true ) {
        for ( auto &n : name ) {
            for ( auto &[ process_name, pid ] : nt::base_process::list( n ) ) {
                if ( process_name.find( n ) != std::wstring::npos ) {
                    return nt::process_kernel { process_name, pid };
                }
            }
        }

        std::this_thread::sleep_for( 100ms );
    }
}

#include <magic_enum.hpp>
#include <iostream>

int main( int argc, const char *argv[] )
{
    using overlay::menu;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );

    if ( std::error_code ec; !fs::exists( "logs" ) && !fs::create_directory( "logs", ec ) ) {
        spdlog::error( xorstr_( "couldn't create logs folder: {}" ), ec.message() );
        ( void ) getchar();
        return 1;
    }

    auto log = spdlog::basic_logger_mt<spdlog::async_factory>( "logger", std::string( xorstr_( "logs/log.txt" ) ), true );
    config::config_system.emplace().init();

    spdlog::info( xorstr_( "waiting for the game" ) );
    log->info( xorstr_( "waiting for the game" ) );

    auto process = wait_process( { L"DeadByDaylight-GRDK-Shipping.exe", L"DeadByDaylight-Win64-Shipping.exe" } );
    if ( !process.attach( PROCESS_ALL_ACCESS ) ) {
        log->error( xorstr_( " error attaching to process (last error is {})" ), GetLastError() );
    }

    if ( !process.base_address() ) {
        log->error( xorstr_( "game has invalid base address" ) );
        return 1;
    }
    spdlog::info( xorstr_( "getting offsets" ) );
    log->info( xorstr_( "getting offsets" ) );

    cheats::offsets.reset( new cheats::offsets_t( process ) );
    if ( !cheats::offsets->get() ) {
        log->error( xorstr_( "error reading offsets" ) );
        return 1;
    }

    // lazy-initialize manager classes
    cheats::actor_manager.emplace( process );
    cheats::esp.emplace( process );
    cheats::utilities.emplace( process );

    std::thread actor_manager_thread( &cheats::actor_manager_t::run_thread, &cheats::actor_manager.value() );
    std::thread utilities_thread( &cheats::utilities_t::run_thread, &cheats::utilities.value() );

    utilities_thread.detach();
    actor_manager_thread.detach();

    log->info( xorstr_( "initializing overlay" ) );
    if ( !menu::get().initialize( L"monero miner 1.0" ) ) {
        log->error( "couldn't initialize menu (last error is {})", GetLastError() );
        return 1;
    }

    menu::get().poll();

    return 0;
}