/*
    RIP GENSHI 14.11.21 - 09.05.22
    IN OUR MEMORIES jk

    dllMain

    Pasted osiris from 11.2021 and updated till big daniel osiris changes + :
    uc users snippers
    osirisAndExtra
    oxide hooks
    seaside
    nezu loader
    TrenenLobby
	aimware Luas
*/
#include <memory>

#ifdef _WIN32
#include <clocale>
#include <Windows.h>
#endif

#include "Hooks.h"
#include <iostream>

#ifdef _WIN32

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    if (!_CRT_INIT(moduleHandle, reason, reserved))
        return FALSE;

    if (reason == DLL_PROCESS_ATTACH) {
        std::setlocale(LC_CTYPE, ".utf8");
#if defined(_DEBUG)
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        SetConsoleCP(936);
        SetConsoleOutputCP(936);
        system("color 09");
        system("title azurre.dll");
//        system("mode CON COLS=110 LINES=25");
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        std::cout <<
            "          _____                    _____                    _____                    _____                    _____                    _____           " << std::endl <<
            "         /\\    \\                  /\\    \\                  /\\    \\                  /\\    \\                  /\\    \\                  /\\    \\          " << std::endl <<
            "        /::\\    \\                /::\\    \\                /::\\____\\                /::\\    \\                /::\\    \\                /::\\    \\         " << std::endl <<
            "       /::::\\    \\               \\:::\\    \\              /:::/    /               /::::\\    \\              /::::\\    \\              /::::\\    \\        " << std::endl <<
            "      /::::::\\    \\               \\:::\\    \\            /:::/    /               /::::::\\    \\            /::::::\\    \\            /::::::\\    \\       " << std::endl <<
            "     /:::/\\:::\\    \\               \\:::\\    \\          /:::/    /               /:::/\\:::\\    \\          /:::/\\:::\\    \\          /:::/\\:::\\    \\      " << std::endl <<
            "    /:::/__\\:::\\    \\               \\:::\\    \\        /:::/    /               /:::/__\\:::\\    \\        /:::/__\\:::\\    \\        /:::/__\\:::\\    \\     " << std::endl <<
            "   /::::\\   \\:::\\    \\               \\:::\\    \\      /:::/    /               /::::\\   \\:::\\    \\      /::::\\   \\:::\\    \\      /::::\\   \\:::\\    \\    " << std::endl <<
            "  /::::::\\   \\:::\\    \\               \\:::\\    \\    /:::/    /      _____    /::::::\\   \\:::\\    \\    /::::::\\   \\:::\\    \\    /::::::\\   \\:::\\    \\   " << std::endl <<
            " /:::/\\:::\\   \\:::\\    \\               \\:::\\    \\  /:::/____/      /\\    \\  /:::/\\:::\\   \\:::\\____\\  /:::/\\:::\\   \\:::\\____\\  /:::/\\:::\\   \\:::\\    \\  " << std::endl <<
            "/:::/  \\:::\\   \\:::\\____\\_______________\\:::\\____\\|:::|    /      /::\\____\\/:::/  \\:::\\   \\:::|    |/:::/  \\:::\\   \\:::|    |/:::/__\\:::\\   \\:::\\____\\ " << std::endl <<
            "\\::/    \\:::\\  /:::/    /\\::::::::::::::::::/    /|:::|____\\     /:::/    /\\::/   |::::\\  /:::|____|\\::/   |::::\\  /:::|____|\\:::\\   \\:::\\   \\::/    / " << std::endl <<
            " \\/____/ \\:::\\/:::/    /  \\::::::::::::::::/____/  \\:::\\    \\   /:::/    /  \\/____|:::::\\/:::/    /  \\/____|:::::\\/:::/    /  \\:::\\   \\:::\\   \\/____/  " << std::endl <<
            "          \\::::::/    /    \\:::\\~~~~\\~~~~~~         \\:::\\    \\ /:::/    /         |:::::::::/    /         |:::::::::/    /    \\:::\\   \\:::\\    \\      " << std::endl <<
            "           \\::::/    /      \\:::\\    \\               \\:::\\    /:::/    /          |::|\\::::/    /          |::|\\::::/    /      \\:::\\   \\:::\\____\\     " << std::endl <<
            "           /:::/    /        \\:::\\    \\               \\:::\\__/:::/    /           |::| \\::/____/           |::| \\::/____/        \\:::\\   \\::/    /     " << std::endl <<
            "          /:::/    /          \\:::\\    \\               \\::::::::/    /            |::|  ~|                 |::|  ~|               \\:::\\   \\/____/      " << std::endl <<
            "         /:::/    /            \\:::\\    \\               \\::::::/    /             |::|   |                 |::|   |                \\:::\\    \\          " << std::endl <<
            "        /:::/    /              \\:::\\____\\               \\::::/    /              \\::|   |                 \\::|   |                 \\:::\\____\\         " << std::endl <<
            "        \\::/    /                \\::/    /                \\::/____/                \\:|   |                  \\:|   |                  \\::/    /         " << std::endl <<
            "         \\/____/                  \\/____/                  ~~                       \\|___|                   \\|___|                   \\/____/          " << std::endl << std::endl <<

        std::endl << "You are using DEBUG version of Azurre !!!" << std::endl <<
        std::endl << "This version may contains features that can crash your game or are not finished yet, Use at ur own risk!" <<
        std::endl << "~Azurre " << std::endl << std::endl;
        Beep(1000, 100);
#endif
        auto window = FindWindowW(L"Valve001", NULL);
        FLASHWINFO flash{ sizeof(FLASHWINFO), window, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
        FlashWindowEx(&flash);
        ShowWindow(window, SW_RESTORE);

        hooks.emplace<Hooks>(moduleHandle);
    }
    return TRUE;
}

#else

void __attribute__((constructor)) DllEntryPoint()
{
    hooks.emplace(Hooks{});
}

#endif
