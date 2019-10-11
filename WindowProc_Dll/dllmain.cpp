// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <locale>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <windows.h>
#include <winuser.h>

HHOOK hook_;

HMODULE current_instance;

LRESULT CALLBACK GetMsgProc(
	_In_ int    code,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	spdlog::info("code:{} {} {}", code, wParam, lParam);
	return CallNextHookEx(hook_,code, wParam, lParam);
}

WNDPROC old_window_proc;

LRESULT CALLBACK new_window_proc(HWND hwnd, UINT code, WPARAM wParam, LPARAM lParam)
{
	//if (code >= 0x200 && code <= 0x20F) {
	//	spdlog::info("code:{} {} {}", code, wParam, lParam);
	//	return 1;
	//}
	//if (code >= 0xA0 && code <= 0xAF)
	//{
	//	spdlog::info("code:{} {} {}", code, wParam, lParam);
	//	return 1;
	//}

	if (code == WM_POINTERDOWN)
	{
		//spdlog::info("code:{} {} {}", code, (void*)wParam, (void*)lParam);
		return 0;
	}

	if (code == WM_POINTERUP)
	{
		spdlog::info("code:{} {} {}", code, (void*)wParam, (void*)lParam);
		//return 0;
	}
	
	return CallWindowProcA(old_window_proc, hwnd, code, wParam, lParam);
}

void init()
{
	BOOL rc = ::AllocConsole();

	if (rc) {
		HANDLE hin = ::GetStdHandle(STD_INPUT_HANDLE);
		HANDLE hout = ::GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		GetConsoleScreenBufferInfo(hout, &coninfo);
		coninfo.dwSize.Y = 1500;
		SetConsoleScreenBufferSize(hout, coninfo.dwSize);


		int hCrt = _open_osfhandle((intptr_t)hin, _O_TEXT);

		if (hCrt != -1) {
			FILE* fpin = _fdopen(hCrt, "r");

			if (fpin)* stdin = *fpin;
		}

		hCrt = _open_osfhandle((intptr_t)hout, _O_TEXT);

		if (hCrt != -1) {
			FILE* fpout = _fdopen(hCrt, "wt");

			if (fpout)* stdout = *fpout;
		}

		hCrt = _open_osfhandle((intptr_t)::GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);

		if (hCrt != -1) {
			FILE* hf = ::_fdopen(hCrt, "w");

			if (hf)* stderr = *hf;
		}

		EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_DISABLED);

		std::ios_base::sync_with_stdio();      // 将iostream 流同c runtime lib 的stdio 同步，标准是同步的
		std::locale::global(std::locale(""));
		//setlocale(LC_CTYPE, "");    // MinGW gcc.
		std::wcout.imbue(std::locale(""));
	}

	spdlog::flush_on(spdlog::level::debug);
	spdlog::set_level(spdlog::level::debug);

	std::thread([]() {

		auto hwnd = FindWindow(L"Qt5QWindowOwnDCIcon", L"Topaz Gigapixel AI v4.0.0");
		while (!(hwnd = FindWindow(L"Qt5QWindowOwnDCIcon", L"Topaz Gigapixel AI v4.0.0")))
		{
			Sleep(1000);
		}
		//auto hwnd = FindWindow(L"Window", L"Cheat Engine 7.0");
		hook_ = SetWindowsHookEx(WH_CALLWNDPROC, GetMsgProc, current_instance, GetWindowThreadProcessId(hwnd, 0));
		spdlog::info("GetLastError : {}", GetLastError());

		old_window_proc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)new_window_proc);
		spdlog::info("HHOOK : {}", (void*)hook_);
		spdlog::info("hwnd : {}", (void*)hwnd);
		spdlog::info("old proc:{}", (void*)old_window_proc);
		spdlog::info("主线程id:{}", GetWindowThreadProcessId(hwnd, 0));

		auto a = SendMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(200, 200));
		std::cout << GetLastError() << std::endl;
		Sleep(100);
		SendMessageA(hwnd, WM_LBUTTONUP, NULL, MAKELPARAM(200, 200));

		std::thread([=]()
			{
				auto inject_ptr = InitializeTouchInjection(10, TOUCH_FEEDBACK_DEFAULT);

				for (;;) {

					if (old_window_proc) {
						POINTER_TOUCH_INFO info = { 0 };
						memset(&info, 0, sizeof(POINTER_TOUCH_INFO));
						info.pointerInfo.pointerType = PT_TOUCH;
						info.pointerInfo.pointerId = 0;
						//info.pointerInfo.hwndTarget = hwnd;
						info.pointerInfo.ptPixelLocationRaw.x = 400;
						info.pointerInfo.ptPixelLocationRaw.y = 400;
						info.pointerInfo.pointerFlags = POINTER_FLAG_DOWN;

						info.pointerInfo.pointerId = 1;
						info.pointerInfo.dwTime = 100;
						
						InjectTouchInput(1, &info);
						
						//SendMessageA( hwnd, WM_POINTERDOWN, 0x00000010, 0x1f90313);
						spdlog::error("{}", GetLastError());

						info.pointerInfo.pointerFlags = POINTER_FLAG_UP;
						InjectTouchInput(1, &info);
						//SendMessageA( hwnd, WM_POINTERUP, 0, 0x1f90313);
					}
					Sleep(3000);
				}
			}).detach();
		}).detach();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	current_instance = hModule;
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: {
		uint8_t code[] = { 0x33,0xC0,0xC3 };
		auto ptr = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "IsDebuggerPresent");
		VirtualProtect(ptr, 3, PAGE_EXECUTE_READWRITE, 0);
		WriteProcessMemory(GetCurrentProcess(), ptr, code, 3, 0);	}
		init();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

__declspec(dllexport) void inject()
{
	
}