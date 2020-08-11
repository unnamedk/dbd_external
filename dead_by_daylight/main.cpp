#include <spdlog/sinks/basic_file_sink.h>
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
#include "cheats/aimbot.hpp"
#include "cheats/meta.hpp"
#include "config/config.hpp"
//#include <curl/curl.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

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

/*size_t checksum_callback( void *content, size_t sz, size_t nmemb, void *userp )
{
    size_t real_size = sz * nmemb;
    auto data = reinterpret_cast<std::string *>( userp );

    data->append( static_cast<char *>( content ), real_size + 1 );
    return real_size;
}

bool try_checksum()
{
    auto hnd = curl_easy_init();
    if ( !hnd ) {
        return false;
    }

    std::string checksum_result;

    curl_easy_setopt( hnd, CURLOPT_BUFFERSIZE, 102400L );
    curl_easy_setopt( hnd, CURLOPT_URL, xorstr_( "https://www.dropbox.com/s/92eza0c76ol0z21/checksum?dl=1" ) );
    curl_easy_setopt( hnd, CURLOPT_NOPROGRESS, 1L );
    curl_easy_setopt( hnd, CURLOPT_FOLLOWLOCATION, 1L );
    curl_easy_setopt( hnd, CURLOPT_USERAGENT, xorstr_( "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36" ) );
    curl_easy_setopt( hnd, CURLOPT_MAXREDIRS, 50L );
    curl_easy_setopt( hnd, CURLOPT_TCP_KEEPALIVE, 1L );
    curl_easy_setopt( hnd, CURLOPT_WRITEFUNCTION, checksum_callback );
    curl_easy_setopt( hnd, CURLOPT_WRITEDATA, &checksum_result );

    auto ret = curl_easy_perform( hnd );
    if ( ret == CURLE_OK ) {
        return checksum_result.find( xorstr_( "20202301" ) ) != std::string::npos;
    }

    curl_easy_cleanup( hnd );

    return false;
}*/

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

    /*if ( !try_checksum() ) {
        log->error( "checksum error!" );
        return 0;
    }*/

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

    if ( !cheats::meta::init() ) {
        log->critical( xorstr_( "no item meta-information file found!" ) );
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
    cheats::aimbot.emplace( process );

    std::thread actor_manager_thread( &cheats::actor_manager_t::run_thread, &cheats::actor_manager.value() );
    std::thread utilities_thread( &cheats::utilities_t::run_thread, &cheats::utilities.value() );

    utilities_thread.detach();
    actor_manager_thread.detach();

    log->info( xorstr_( "initializing overlay" ) );
    if ( !menu::get().initialize( L"monero miner 1.0" ) ) {
        log->error( xorstr_( "couldn't initialize menu (last error is {})" ), GetLastError() );
        return 1;
    }

    menu::get().poll();

    return 0;
}