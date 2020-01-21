#include "misc.hpp"
#include <phnt_windows.h>

bool utils::is_key_pressed( std::uint16_t code ) noexcept
{
    return ( code == 0 ) || ( GetAsyncKeyState( code ) & 0x8000 );
}

void utils::press_key( std::uint16_t code, std::uint32_t time ) noexcept
{
    INPUT i {};
    i.type = INPUT_KEYBOARD;
    i.ki.wScan = MapVirtualKey( code, MAPVK_VK_TO_VSC );
    i.ki.time = time;
    i.ki.dwExtraInfo = 0;
    i.ki.wVk = code;
    i.ki.dwFlags = KEYEVENTF_SCANCODE;

    SendInput( 1, &i, sizeof( INPUT ) );
    i.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    SendInput( 1, &i, sizeof( INPUT ) );
}