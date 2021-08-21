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

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	DWORD old_protect;

	if (fdwReason == DLL_PROCESS_ATTACH) {
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
