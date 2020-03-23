#include "pch.h"

#ifdef _WIN64

#include "LdrLoadDll.h"
#pragma comment (lib, "Psapi.lib")

BYTE LdrLoadDll_Shell_WOW64[] = 
{ 
	0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08, 0x85, 0xF6, 0x75, 0x0A, 0xB8, 0x01, 0x00, 0x20, 0x00, 0x5E, 0x5D, 0xC2, 0x04, 0x00, 0x8B, 0x4E, 0x04, 0x85, 0xC9, 0x75, 0x0A, 0xB8, 0x02, 0x00, 0x20, 0x00, 0x5E, 0x5D, 0xC2, 
	0x04, 0x00, 0x8D, 0x46, 0x14, 0x56, 0x89, 0x46, 0x10, 0x8D, 0x46, 0x0C, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0xFF, 0xD1, 0x33, 0xC9, 0x89, 0x46, 0x08, 0x85, 0xC0, 0xBA, 0x03, 0x00, 0x20, 0x00, 0x5E, 0x0F, 0x48, 0xCA, 0x8B, 
	0xC1, 0x5D, 0xC2, 0x04, 0x00
};

DWORD _LdrLoadDll_WOW64(const wchar_t * szDllFile, HANDLE hTargetProc, LAUNCH_METHOD Method, DWORD Flags, HINSTANCE & hOut, DWORD & LastWin32Error)
{
	LDR_LOAD_DLL_DATA_WOW64 data{ 0 };
	data.pModuleFileName.MaxLength = sizeof(data.Data);

	size_t size_out = 0;
	if (FAILED(StringCbLengthW(szDllFile, data.pModuleFileName.MaxLength, &size_out)))
	{
		return INJ_ERR_STRINGC_XXX_FAIL;
	}

	if (FAILED(StringCbCopyW(ReCa<wchar_t *>(data.Data), data.pModuleFileName.MaxLength, szDllFile)))
	{
		return INJ_ERR_STRINGC_XXX_FAIL;
	}

	data.pModuleFileName.Length = (WORD)size_out;

	void * pLdrLoadDll = nullptr;
	if (!GetProcAddressEx_WOW64(hTargetProc, TEXT("ntdll.dll"), "LdrLoadDll", pLdrLoadDll))
	{
		LastWin32Error = GetLastError();

		return INJ_ERR_LDRLOADDLL_MISSING;
	}
	data.pLdrLoadDll = MDWD(pLdrLoadDll);

	ULONG_PTR ShellSize = sizeof(LdrLoadDll_Shell_WOW64);

	BYTE * pAllocBase	= ReCa<BYTE*>(VirtualAllocEx(hTargetProc, nullptr, sizeof(LDR_LOAD_DLL_DATA_WOW64) + ShellSize + 0x10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	BYTE * pArg			= pAllocBase;
	BYTE * pShell		= ReCa<BYTE*>(ALIGN_UP(ReCa<ULONG_PTR>(pArg + sizeof(LDR_LOAD_DLL_DATA_WOW64)), 0x10));
	
	if (!pArg)
	{
		LastWin32Error = GetLastError();

		return INJ_ERR_OUT_OF_MEMORY_EXT;
	}

	if (!WriteProcessMemory(hTargetProc, pArg, &data, sizeof(LDR_LOAD_DLL_DATA_WOW64), nullptr))
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pAllocBase, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	if (!WriteProcessMemory(hTargetProc, pShell, LdrLoadDll_Shell_WOW64, ShellSize, nullptr))
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pAllocBase, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	DWORD remote_ret = 0;
	DWORD dwRet = StartRoutine_WOW64(hTargetProc, MDWD(pShell), MDWD(pArg), Method, (Flags & INJ_THREAD_CREATE_CLOAKED) != 0, LastWin32Error, remote_ret);
	
	if (dwRet != SR_ERR_SUCCESS)
	{
		if (Method != LAUNCH_METHOD::LM_QueueUserAPC && !(Method == LAUNCH_METHOD::LM_HijackThread && dwRet == SR_HT_ERR_REMOTE_TIMEOUT))
		{
			VirtualFreeEx(hTargetProc, pAllocBase, 0, MEM_RELEASE);
		}

		return dwRet;
	}
	else if (remote_ret != INJ_ERR_SUCCESS)
	{
		if (Method != LAUNCH_METHOD::LM_QueueUserAPC)
		{
			VirtualFreeEx(hTargetProc, pAllocBase, 0, MEM_RELEASE);
		}

		return remote_ret;
	}

	if (!ReadProcessMemory(hTargetProc, pAllocBase, &data, sizeof(LDR_LOAD_DLL_DATA_WOW64), nullptr))
	{
		LastWin32Error = GetLastError();

		if (Method != LAUNCH_METHOD::LM_QueueUserAPC)
		{
			VirtualFreeEx(hTargetProc, pAllocBase, 0, MEM_RELEASE);
		}

		return INJ_ERR_VERIFY_RESULT_FAIL;
	}

	if (Method != LAUNCH_METHOD::LM_QueueUserAPC)
	{
		VirtualFreeEx(hTargetProc, pAllocBase, 0, MEM_RELEASE);
	}

	hOut = ReCa<HINSTANCE>(MPTR(data.hRet));

	return INJ_ERR_SUCCESS;
}

#endif