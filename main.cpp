/*
A hack for Wyrmsun that will automatically create worker units out of the currently selected structure when a player's gold is over 3000. 
It accomplishes this by filling the current unit buffer with worker data and then calling the create unit function in the game.

After injecting this hack, go in game and recruit a worker. Then select a structure as you collect gold. 
You will notice workers being queued automatically.

Due to the way Wyrmsun handles recruitment, it is possible to create units out of whatever is selected, including other units.

The technique and offsets used are discussed here: https://gamehacking.academy/lesson/41
*/
#include <Windows.h>

HANDLE wyrmsun_base;

DWORD* base;
DWORD* unitbase;
DWORD recruit_unit_ret_address;
DWORD recruit_unit_call_address;
unsigned char unitdata[0x110];
bool init = false;

DWORD gameloop_ret_address;
DWORD gameloop_call_address;
DWORD *gold_base, *gold;

// The recruit unit codecave hooks the game's recruit unit function
// It's main job is to copy a valid buffer of data for a worker unit
// instead of having to reverse the structure
__declspec(naked) void recruit_unit_codecave() {
    __asm {
        pushad
        mov base, ecx
    }

    unitbase = (DWORD*)(*base);
    memcpy(unitdata, unitbase, 0x110);
    init = true;

    _asm {
        popad
        push ecx
        mov ecx, esi
        call recruit_unit_call_address
        jmp recruit_unit_ret_address
    }
}

// In the main game loop, our codecave will check the current player's gold
// If it is over 3000, and we have a valid worker buffer, call the recruit unit function
// with worker data.
__declspec(naked) void gameloop_codecave() {
    __asm {
        pushad
    }

    gold_base = (DWORD*)((DWORD)wyrmsun_base + 0x0061A504);
    gold = (DWORD*)(*gold_base + 0x78);
    gold = (DWORD*)(*gold + 4);
    gold = (DWORD*)(*gold + 8);
    gold = (DWORD*)(*gold + 4);
    gold = (DWORD*)(*gold);
    gold = (DWORD*)(*gold + 0x14);

    if (init && *gold > 3000) {
        memcpy(unitbase, unitdata, 0x110);
        __asm {
            mov ecx, base
            push ecx
            call recruit_unit_call_address
        }
    }


    __asm {
        popad
        call gameloop_call_address
        jmp gameloop_ret_address
    }
}

// When our DLL is attached, unprotect the memory at the code we wish to write at
// Then set the first opcode to E9, or jump
// Caculate the location using the formula: new_location - original_location+5
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	DWORD old_protect;

	if (fdwReason == DLL_PROCESS_ATTACH) {
		// Since Wyrmsun loads code dynamically, we need to calculate offsets based of the base address of the main module
		wyrmsun_base = GetModuleHandle(L"wyrmsun.exe");

		unsigned char* hook_location = (unsigned char*)((DWORD)wyrmsun_base + 0x223471);
		recruit_unit_ret_address = (DWORD)hook_location + 8;
		recruit_unit_call_address = (DWORD)wyrmsun_base + 0x2CF7;

		VirtualProtect((void*)hook_location, 8, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&recruit_unit_codecave - ((DWORD)hook_location + 5);
		*(hook_location + 5) = 0x90;
		*(hook_location + 6) = 0x90;
		*(hook_location + 7) = 0x90;

		hook_location = (unsigned char*)((DWORD)wyrmsun_base + 0x385D34);
		gameloop_ret_address = (DWORD)hook_location + 5;
		gameloop_call_address = (DWORD)wyrmsun_base + 0xDBCA;

		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&gameloop_codecave - ((DWORD)hook_location + 5);
	}

	return true;
}
