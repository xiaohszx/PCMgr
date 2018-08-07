// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "resource.h"
#include "sysfuns.h"

HINSTANCE hInst;

extern HICON HIconDef;
extern HCURSOR hCurLoading;
extern MEMORYSTATUSEX memory_statuex;

extern BOOL LoadDll();
extern void FreeDll();
extern void WindowEnumStart();
extern void WindowEnumDestroy();
extern void MPERF_FreeCpuInfos();
extern void MLG_SetLanuageItems_Destroy();
extern void MLG_Startup();
extern BOOL MGetExeFileTrust(LPCWSTR lpFileName);
extern BOOL MUnInitMyDbgView();
extern void M_CFG_Init();
extern BOOL MUnInitKernelNTPDB();
extern _GetModuleFileNameW dGetModuleFileNameW;

M_CAPI(int) MPERF_GetProcessNumber();
extern void ShowMainCoreStartUp();

void DllDestroy();
void DllStartup();
void DllAnti();
bool DllAntiCheckVery();
void DllAntiCrush();

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:	
		hInst = (HINSTANCE)hModule;
		DllStartup();
		DllAnti();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DllDestroy();
		break;
	}
	return TRUE;
}

void DllStartup() {
	HIconDef = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICONDEFAPP), IMAGE_ICON, 16, 16, 0);
	LoadDll();
	M_CFG_Init();
	MLG_Startup();
	hCurLoading = LoadCursor(NULL, IDC_WAIT);
	ShowMainCoreStartUp();
	MPERF_GetProcessNumber();
	WindowEnumStart();
	memory_statuex.dwLength = sizeof(memory_statuex);
}
void DllDestroy() {
	hInst = NULL;
	MUnInitKernelNTPDB();
	MLG_SetLanuageItems_Destroy();
	MPERF_FreeCpuInfos();
	WindowEnumDestroy();
	MUnInitMyDbgView();
	FreeDll();
}
void DllAnti() 
{
	bool needCrush = false;
	//if (!DllAntiCheckVery())
	//	needCrush = true;

	//WCHAR fileName[MAX_PATH];
	//dGetModuleFileNameW(NULL, fileName, MAX_PATH);
	//if (MGetExeFileTrust(fileName))
	//	needCrush = true;

	if (needCrush) 
		DllAntiCrush();
}
bool DllAntiCheckVery()
{
	//WCHAR fileName[MAX_PATH];
	//dGetModuleFileNameW(hInst, fileName, MAX_PATH);
	//if (MGetExeFileTrust(fileName))
		return true;
	//return false;
}
void DllAntiCrush()
{
	DllDestroy();
}