#pragma once

#include "Injection.h"

#define RELOC_FLAG86(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)
#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG86
#endif

using f_LoadLibraryA		= decltype(LoadLibraryA);
using f_GetModuleHandleA	= decltype(GetModuleHandleA);
using f_VirtualAlloc		= decltype(VirtualAlloc);
using f_GetProcAddress		= ULONG_PTR	(WINAPI*)(HINSTANCE hModule, const char * lpProcName);
using f_DLL_ENTRY_POINT		= BOOL		(WINAPI*)(void * hDll, DWORD dwReason, void * pReserved);

struct MANUAL_MAPPER
{
	~MANUAL_MAPPER();

	BYTE * pRawData;

	BYTE * pLocalImageBase;
	IMAGE_DOS_HEADER		* pLocalDosHeader;
	IMAGE_NT_HEADERS		* pLocalNtHeaders;
	IMAGE_OPTIONAL_HEADER	* pLocalOptionalHeader;
	IMAGE_FILE_HEADER		* pLocalFileHeader;
	
	DWORD ImageSize;
	DWORD AllocationSize;
	DWORD ShiftOffset;

	BYTE * pAllocationBase;
	BYTE * pTargetImageBase;
	BYTE * pShellcode;
	BYTE * pManualMappingData;

	DWORD ShellSize;

	HANDLE hTargetProcess;
	DWORD Flags;
	bool bKeepTarget;

	DWORD AllocateMemory	(DWORD & LastWin32Error);
	DWORD CopyData			(DWORD & LastWin32Error);
	DWORD RelocateImage		(DWORD & LastWin32Error);
	DWORD InitSecurityCookie(DWORD & LastWin32Error);
	DWORD CopyImage			(DWORD & LastWin32Error);
	DWORD SetPageProtections(DWORD & LastWin32Error);
};

struct MANUAL_MAPPING_DATA
{
	HINSTANCE								hRet;
	HINSTANCE								hK32;
	f_LoadLibraryA						*	pLoadLibraryA;
	f_GetModuleHandleA					*	pGetModuleHandleA;
	f_GetProcAddress						pGetProcAddress;
	f_RtlInsertInvertedFunctionTable		pRtlInsertInvertedFunctionTable;
	f_VirtualAlloc						*	pVirtualAlloc;
	BYTE								*	pModuleBase;
	DWORD									Flags;
};

#ifdef _WIN64

struct MANUAL_MAPPER_WOW64
{
	~MANUAL_MAPPER_WOW64();

	BYTE * pRawData;

	BYTE * pLocalImageBase;
	IMAGE_DOS_HEADER		* pLocalDosHeader;
	IMAGE_NT_HEADERS32		* pLocalNtHeaders;
	IMAGE_OPTIONAL_HEADER32	* pLocalOptionalHeader;
	IMAGE_FILE_HEADER		* pLocalFileHeader;

	DWORD ImageSize;
	DWORD AllocationSize;
	DWORD ShiftOffset;

	BYTE * pAllocationBase;
	BYTE * pTargetImageBase;
	BYTE * pShellcode;
	BYTE * pManualMappingData;

	DWORD ShellSize;

	HANDLE hTargetProcess;
	DWORD Flags;
	bool bKeepTarget;

	DWORD AllocateMemory	(DWORD & LastWin32Error);
	DWORD CopyData			(DWORD & LastWin32Error);
	DWORD RelocateImage		(DWORD & LastWin32Error);
	DWORD InitSecurityCookie(DWORD & LastWin32Error);
	DWORD CopyImage			(DWORD & LastWin32Error);
	DWORD SetPageProtections(DWORD & LastWin32Error);
};

struct MANUAL_MAPPING_DATA_WOW64
{
	DWORD hRet;
	DWORD hK32;
	DWORD pLoadLibraryA;
	DWORD pGetModuleHandleA;
	DWORD pGetProcAddress;
	DWORD pRtlInsertInvertedFunctionTable;
	DWORD pVirtualAlloc;
	DWORD pModuleBase;
	DWORD Flags;
};

#endif