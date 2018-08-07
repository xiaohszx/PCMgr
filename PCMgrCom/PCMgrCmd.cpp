// PCMgrCom.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "PCMgrCmd.h"
#include "StringSplit.h"
#include <locale>
#include <string>
#include "..\TaskMgrCore\syshlp.h"
#include "..\TaskMgrCore\mapphlp.h"
#include "..\TaskMgrCore\vprocx.h"
#include "..\TaskMgrCore\suact.h"
#include "..\TaskMgrCore\prochlp.h"
#include "..\TaskMgrCore\prochlp.h"
#include "..\TaskMgrCore\thdhlp.h"
#include "..\TaskMgrCore\fmhlp.h"
#include "..\TaskMgrCore\lghlp.h"
#include "..\TaskMgrCore\loghlp.h"
#include "..\TaskMgrCore\kernelhlp.h"
#include "..\TaskMgrCore\ntsymbol.h"
#include "..\TaskMgrCore\PathHelper.h"

using namespace std;

int curPos = 0;
HANDLE hOutput = NULL;
char maxpath[MAX_PATH];
WCHAR appdir[MAX_PATH];

void MRunCmd_UnLoadKernel(std::vector<std::string>* cmds, int size);

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	MLG_SetLanuageRes((LPWSTR)L"", (LPWSTR)L"zh");
	MLG_SetLanuageItems_NoRealloc();
	M_LOG_Init_InConsole();

	strcpy_s(maxpath, argv[0]);
	LPWSTR wmaxpath = MConvertLPCSTRToLPWSTR(maxpath);
	wstring *pathdir = Path::GetDirectoryName(wmaxpath);
	Log(L"App Dir : %s", pathdir->c_str());
	wcscpy_s(appdir, pathdir->c_str());
	MConvertStrDel(wmaxpath);
	MConvertStrDel(pathdir);

	if (argc > 1)
		return MRunArgs(argc, argv);
	else return MRunMain();
}

int MRunArgs(int argc, char *argv[])
{
	if (MStrEqualA(argv[1], "taskkill"))
	{
		if (argc > 2 && !MStrEqualA(argv[2], ""))
		{
			MAppHideCos();
			DWORD pid = static_cast<DWORD>(atoll(argv[2]));
			
			if (argc > 3 && MStrEqualA(argv[3], "force"))
			{
				BOOL useApc = FALSE;
				if (argc > 3 && MStrEqualA(argv[3], "apc"))useApc = TRUE;
				NTSTATUS status = 0;
				if (!M_SU_TerminateProcessPID(pid, 0, &status, useApc))
					printf("TerminateProcess Failed 0x%08X", status);
				else if (status == STATUS_ACCESS_DENIED)
					printf("STATUS_ACCESS_DENIED");
				else if (status == STATUS_INVALID_HANDLE || status == 0xC000000B)
					printf("STATUS_INVALID_HANDLE");
				else if (status == STATUS_SUCCESS)
					return 0;
				else printf("TerminateProcess Failed 0x%08X", status);
			}
			else 
			{
				HANDLE hProcess;
				NTSTATUS status = MOpenProcessNt(pid, &hProcess);
				if (status == STATUS_SUCCESS)
				{
					status = MTerminateProcessNt(0, hProcess);
					if (status == STATUS_ACCESS_DENIED)
						printf("STATUS_ACCESS_DENIED");
					else if (status == STATUS_SUCCESS)
						return 0;
				}
				else if (status == STATUS_ACCESS_DENIED)
					printf("STATUS_ACCESS_DENIED");
				else if (status == STATUS_INVALID_HANDLE || status == 0xC000000B)
					printf("STATUS_INVALID_HANDLE");
				else printf("OpenProcess Failed 0x%08X", status);
			}

			MAppHideCos();
			system("PAUSE");
		}
	}
	else if (MStrEqualA(argv[1], "vmodules"))
	{
		if (argc > 2 && !MStrEqualA(argv[2], ""))
		{
			MAppHideCos();
			DWORD pid = static_cast<DWORD>(atoll(argv[2]));
			MAppVProcessModuls(pid, NULL, (LPWSTR)L"");
		}
	}
	else if (MStrEqualA(argv[1], "unloadk"))
	{
		if (!MIsRunasAdmin()) {
			MAppHideCos();
			MAppRebotAdmin2((LPWSTR)L"unloadk");
		}
		else MRunCmd_UnLoadKernel(NULL, 0);
	}
	return 0;
}

int MPrintInfo()
{
	printf_s("PCMgr Command Line Tool\n");
	printf_s("Version : 1.0.0.1\n\n");
	MGetWindowsBulidVersion();
	printf_s("\n");
	return 0;
}

int MRunMain()
{
	MPrintInfo();

REENTER:
	printf_s("\n>");

	/*

	//COORD coord; //屏幕上的坐标
	//CONSOLE_SCREEN_BUFFER_INFO csbi; //控制台屏幕缓冲区信息
	int ch = 0;
	while (ch == 0xE0 || ch == 127)
	{
		putchar(ch);
		ch = getchar();
		if (ch == 127)//DEL
			putchar('\b');
		else if (ch == 0xE0) {
			GetConsoleScreenBufferInfo(hOutput, &csbi);
			coord.X = csbi.dwCursorPosition.X; //得到坐标X的值
			coord.Y = csbi.dwCursorPosition.Y; //得到坐标Y的值
			if (ch == 0x48)//上
			{
				if (coord.Y != 0)coord.Y--;
			}
			else if (ch == 0x50)//下
			{
				coord.Y++;
			}
			else if (ch == 0x4b)//左
			{
				if (coord.X != 0) { coord.X--; }
			}
			else if (ch == 0x4d)//右
			{
				if (coord.X != 79)coord.X++;
				else
				{
					coord.X = 0;
					coord.Y++;
				}
			}
			SetConsoleCursorPosition(hOutput, coord);
		}
		else if (ch == 0x0d) break;
	}*/

	char maxbuf[260];
	gets_s(maxbuf);

	if (maxbuf[0] == '>')
		maxbuf[0] = ' ';
	if (MRunCmd(maxbuf))
		return 0;

	goto REENTER;

	return 0;
}

int MPrintMumberWithLen(DWORD n, size_t len)
{
	char munbuf[16];
	sprintf_s(munbuf, "%d", n);
	size_t munstrlen = strlen(munbuf);
	if (munstrlen < len)
	{
		size_t outlen = len - munstrlen;
		for (size_t i = 0; i < outlen; i++)putchar(' ');
		for (size_t i = 0; i < 16 && i + outlen < len; i++)
			putchar(munbuf[i]);
		return static_cast<int>(outlen);
	}
	else printf(munbuf);
	return static_cast<int>(len);
}
int MPrintStrWithLenW(LPWSTR s, size_t len)
{
	if (s != NULL) {
		wprintf_s(L"%s", s);
		size_t slen = wcslen(s);
		if (slen > 0) 
		{
			size_t soutlen = len - (size_t)slen;
			if (soutlen > 0)
				for (size_t i = 0; i < soutlen; i++)
					putchar(' ');
			return static_cast<int>(len - slen);
		}
	}
	return 0;
}
int MPrintStrWithLenA(LPCSTR s, size_t len)
{
	if (s != NULL) {
		printf_s("%s", s);
		size_t slen = strlen(s);
		if (slen > 0)
		{
			size_t soutlen = len - (size_t)slen;
			if (soutlen > 0)
				for (size_t i = 0; i < soutlen; i++)
					putchar(' ');
			return static_cast<int>(len - slen);
		}
	}
	return 0;
}
void MPrintSuccess() {
	printf("Success.\n");
}

void __cdecl MEnumProcessCallBack(DWORD pid, DWORD parentid, LPWSTR exename, LPWSTR exefullpath, int tp, HANDLE hProcess)
{
	MPrintMumberWithLen(pid, 5);
	printf(" ");
	MPrintMumberWithLen(parentid, 5);
	printf("        ");//6
	MPrintStrWithLenW(exename, 32);
	wprintf_s(L"   %s\n", exefullpath);
}
void __cdecl MDACALLBACK(ULONG_PTR curaddress, LPWSTR addressstr, LPWSTR shellstr, LPWSTR bariny, LPWSTR asmstr)
{
	wprintf_s(L"%s", addressstr);
	MPrintStrWithLenW(bariny, 30);
	printf("    ");
	MPrintStrWithLenW(shellstr, 30);
	printf("  ");
	MPrintStrWithLenW(asmstr, 30);
	printf("\n");
}
BOOLEAN CALLBACK MENUMSTRUCTOFFESTCallBack(PSYMBOL_INFO psi, ULONG SymSize, PVOID Context)
{
	MPrintStrWithLenA(psi->Name, 32);
#ifdef _AMD64_
	printf(" 0x%I54X", psi->Address);
#else
	printf(" 0x%08X", (ULONG_PTR)psi->Address);
#endif
	return TRUE;
}
BOOLEAN CALLBACK MEnumSymRoutine(PSYMBOL_INFO psi, ULONG SymSize, PVOID Context)
{
	MPrintStrWithLenA(psi->Name, 32);
#ifdef _AMD64_
	printf(" 0x%I54X", psi->Address);
#else
	printf(" 0x%08X", (ULONG_PTR)psi->Address);
#endif
	return TRUE;
}


void MRunCmd_Procs(std::vector<std::string>* cmds, int size)
{
	wprintf_s(L"PID     ParentPID ProcessName                          FullPath\n");
	MEnumProcess((EnumProcessCallBack)MEnumProcessCallBack);
}
void MRunCmd_TaskKill(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter pid.\n"); return; }
	DWORD pid = static_cast<DWORD>(atoll((*cmds)[1].c_str()));
	if (pid > 4) {
		if (size > 2 && (*cmds)[2] == "force")
		{
			BOOL useApc = FALSE;
			if (size > 3 && (*cmds)[3] == "apc")useApc = TRUE;
			NTSTATUS status = 0;
			if (!M_SU_TerminateProcessPID(pid, 0, &status, useApc))
				printf("TerminateProcess Failed 0x%08X\n", status);
			else if (status == STATUS_ACCESS_DENIED)
				printf("Failed : STATUS_ACCESS_DENIED\n");
			else if (status == STATUS_INVALID_HANDLE || status == 0xC000000B)
				printf("Failed : STATUS_INVALID_HANDLE\n");
			else if (status == STATUS_SUCCESS)
				MPrintSuccess();
			else printf("TerminateProcess Failed 0x%08X\n", status);
		}
		else
		{
			HANDLE hProcess;
			NTSTATUS status = MOpenProcessNt(pid, &hProcess);
			if (status == STATUS_SUCCESS)
			{
				status = MTerminateProcessNt(0, hProcess);
				if (status == STATUS_ACCESS_DENIED)
					printf("Failed : STATUS_ACCESS_DENIED\n");
				else if (status == STATUS_SUCCESS)
					printf("Success.\n");
			}
			else if (status == STATUS_ACCESS_DENIED)
				printf("Failed : STATUS_ACCESS_DENIED\n");
			else if (status == STATUS_INVALID_HANDLE || status == 0xC000000B)
				printf("Failed : STATUS_INVALID_HANDLE\n");
			else printf("OpenProcess Failed 0x%08X\n", status);
		}
	}
	else printf("Invalid pid.\n");
}
void MRunCmd_ThreadKill(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter tid.\n"); return; }
	DWORD tid = static_cast<DWORD>(atoll((*cmds)[1].c_str()));
	NTSTATUS status = 0;
	if (size > 2 && (*cmds)[2] == "force")
	{
		BOOL useApc = FALSE;
		if (size > 3 && (*cmds)[3] == "apc")useApc = TRUE;
		if (!(M_SU_TerminateThreadTID(tid, 0, &status, useApc) && (status) == STATUS_SUCCESS))
		{
			if (status == STATUS_INVALID_HANDLE) printf("Failed : STATUS_INVALID_HANDLE\n");
			else if (status == STATUS_ACCESS_DENIED)
				printf("Failed : STATUS_ACCESS_DENIED\n");
			else if (status == STATUS_SUCCESS)
				MPrintSuccess();
			else printf("Failed : TerminateThread : 0x%08X\n", status);
		}
	}
	else {
		HANDLE hThread;
		DWORD NtStatus = MOpenThreadNt(tid, &hThread, tid);
		if (NtStatus == STATUS_INVALID_HANDLE) {
			printf("Failed : STATUS_INVALID_HANDLE\n");
		}
		if (hThread) {
			NtStatus = MTerminateThreadNt(hThread);
			if (NtStatus == STATUS_SUCCESS)
				printf("Success.\n");
			else printf("Failed : TerminateThread : 0x%08X\n", status);
		}
		else printf("Failed : OpenThread : 0x%08X\n", status);
	}
}
void MRunCmd_TaskSuspend(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter pid.\n"); return; }
	DWORD pid = static_cast<DWORD>(atoll((*cmds)[1].c_str()));
	if (pid > 4) {
		NTSTATUS status = MSuspendProcessNt(pid, 0);
		if (status == STATUS_INVALID_HANDLE) printf("Failed : STATUS_INVALID_HANDLE\n");
		else if (status == STATUS_ACCESS_DENIED)
			printf("Failed : STATUS_ACCESS_DENIED\n");
		else if (status == STATUS_SUCCESS)
			MPrintSuccess();
		else printf("Failed : SuspendProcess : 0x%08X\n", status);
	}
	else printf("Invalid pid.\n");
}
void MRunCmd_TaskResume(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter pid.\n"); return; }
	DWORD pid = static_cast<DWORD>(atoll((*cmds)[1].c_str()));
	if (pid > 4) {
		NTSTATUS status = MResumeProcessNt(pid, 0);
		if (status == STATUS_INVALID_HANDLE) printf("Failed : STATUS_INVALID_HANDLE\n");
		else if (status == STATUS_ACCESS_DENIED)
			printf("Failed : STATUS_ACCESS_DENIED\n");
		else if (status == STATUS_SUCCESS)
			MPrintSuccess();
		else printf("Failed : SuspendProcess : 0x%08X\n", status);
	}
	else printf("Invalid pid.\n");
}
void MRunCmd_Test(std::vector<std::string>* cmds, int size) {
	printf("MRunCmd_Test cmd : %s\n", (*cmds)[0].c_str());
}
void MRunCmd_Help(std::vector<std::string>* cmds, int size) {
	printf("Help : \n");
	printf("    tasklist : list all running process\n");
	printf("    taskkill pid [force] [useApc] : kill a running process use process id\n            force : Want to use kernel force kill process\n            useApc : When force kill , should use APC to terminate threads\n");
	printf("    tasksuspend pid [force] : suspend process use process id\n            force : Want to use kernel force suspend process\n");
	printf("    taskresume pid [force] : resume process use process id\n            force : Want to use kernel force resume process\n");
	printf("    threadkill tid [force] [useApc] : kill a thread use thread id\n            force : Want to use kernel force kill thread\n            useApc : When force kill , should use APC to terminate thread\n");
	printf("    loaddrv filepath [servicename] : load a sys driver\n            filepath : The sys file path that you want to load\n            servicename : The driver host service name, it will use when you want unload it, default is sys file name\n");
	printf("    unloaddrv servicename : unload a sys driver\n            servicename : The driver host service name, when you enter in loaddrv command 3rd parameter\n");
	printf("    kda startaddress [disasm size] : Disassembler kernel\n            startaddress : Want to disassembler kernel startadderss in memory(hex)\n            disasm size : The code size you want to disassembler(in bytes and hex), defaut is 0xFF\n");
	printf("    loadk : load pcmgr kernel driver\n");
	printf("    unloadk : unload pcmgr kernel driver\n");
	printf("    toadmin : run pcmgr as adminstrator\n");

}
void MRunCmd_KDA(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter startaddress.\n"); return; }
#ifdef _AMD64_
	ULONG_PTR startaddress = static_cast<ULONG_PTR>(_atoi64((*cmds)[1].c_str()));
#else
	ULONG_PTR startaddress = static_cast<ULONG_PTR>(atol((*cmds)[1].c_str()));
#endif
	ULONG dissize = 0xff;
	if(size > 2) sscanf_s((*cmds)[2].c_str(), "%x", &dissize);
	if (!M_SU_KDA(MDACALLBACK, startaddress, dissize))
		printf("M_SU_KDA failed !");
}
void MRunCmd_LoadKernel(std::vector<std::string>* cmds, int size)
{
	MDoNotStartMyDbgView();
	MInitKernel(appdir);
}
void MRunCmd_UnLoadKernel(std::vector<std::string>* cmds, int size)
{
	if (!MUninitKernel())
		system("PAUSE");
}
void MRunCmd_LoadDriver(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter filepath.\n"); return; }

	LPWSTR filepathw = MConvertLPCSTRToLPWSTR((*cmds)[2].c_str());

	WCHAR scName[32];
	if (size > 2) {
		LPWSTR scNamew = MConvertLPCSTRToLPWSTR((*cmds)[3].c_str());
		wcscpy_s(scName, scNamew);
		MConvertStrDel(scNamew);
	}
	else {
		wstring *scnamedef = Path::GetFileNameWithoutExtension(filepathw);
		wcscpy_s(scName, scnamedef->c_str());
		MConvertStrDel(scnamedef);
	}
	if (!MLoadKernelDriver(scName, filepathw, scName))
		MPrintSuccess();
	MConvertStrDel(filepathw);
}
void MRunCmd_UnLoadDriver(std::vector<std::string>* cmds, int size)
{
	if (size < 2) { printf("Please enter servicename.\n"); return; }

	LPWSTR scNamew = MConvertLPCSTRToLPWSTR((*cmds)[2].c_str());
	if (MUnLoadKernelDriver(scNamew))
		MPrintSuccess();
	MConvertStrDel(scNamew);
}
bool MRunCmd_Rebootasadmin(std::vector<std::string>* cmds, int size)
{
	if(MIsRunasAdmin())
	{
		LogInfo(L"You are alreday run as adminstrator.");
		return false;
	}
	MAppHideCos();
	MAppRebotAdmin2((LPWSTR)L"");
	return true;
}
void MRunCmd_DT(std::vector<std::string>* cmds, int size) {
	if (size < 2) { printf("Please enter a symbol name.\n"); return; }

	if (MKEnumSymStructs(MGetNTBaseAddress(), (*cmds)[2].c_str(), (PSYM_ENUMERATESYMBOLS_CALLBACK)MENUMSTRUCTOFFESTCallBack, NULL))
		MPrintSuccess();
}
void MRunCmd_SYMS(std::vector<std::string>* cmds, int size) {
	if (MKEnumAllSym(MGetNTBaseAddress(), (PSYM_ENUMERATESYMBOLS_CALLBACK)MEnumSymRoutine, NULL))
		MPrintSuccess();
}

#define CMD_CASE(cmd, go) else if (cmds[0] == cmd) go(&cmds, size)
#define CMD_CASE_CAN_EXIT(cmd, go) else if (cmds[0] == cmd) {if(go(&cmds, size))return true;}

bool MRunCmd(char * maxbuf)
{
	string cmd;
	cmd = maxbuf;

	vector<string> cmds;
	SplitString(cmd, cmds, " ");

	int size = static_cast<int>(cmds.size());
	if (size >= 1)
	{
		if (cmds[0] == "exit") return true;
		else if (cmds[0] == "cls") system("cls");
		CMD_CASE("help", MRunCmd_Help);
		CMD_CASE("?", MRunCmd_Help);
		CMD_CASE("tasklist", MRunCmd_Procs);
		CMD_CASE("taskkill", MRunCmd_TaskKill);
		CMD_CASE("tasksuspend", MRunCmd_TaskSuspend);
		CMD_CASE("taskresume", MRunCmd_TaskResume);
		CMD_CASE("threadkill", MRunCmd_ThreadKill);
		CMD_CASE("kda", MRunCmd_KDA);
		CMD_CASE("loadk", MRunCmd_LoadKernel);
		CMD_CASE("unloadk", MRunCmd_UnLoadKernel);
		CMD_CASE("unloaddrv", MRunCmd_UnLoadDriver);
		CMD_CASE("loaddrv", MRunCmd_LoadDriver);
		CMD_CASE("test", MRunCmd_Test);
		CMD_CASE("dt", MRunCmd_DT);
		CMD_CASE("syms", MRunCmd_SYMS);
		CMD_CASE_CAN_EXIT("toadmin", MRunCmd_Rebootasadmin)
		else if (MFM_FileExistA(cmds[0].c_str())) {
			LPWSTR wmaxpath = MConvertLPCSTRToLPWSTR((LPCSTR)cmds[0].c_str());
			MFM_OpenAFile(wmaxpath);
			MConvertStrDel(wmaxpath);
		}
		else if (MFM_FileExistA(("%SystemRoot%\\" + cmds[0]).c_str())) {
			LPWSTR wmaxpath = MConvertLPCSTRToLPWSTR((LPCSTR)("%SystemRoot%\\" + cmds[0]).c_str());
			MFM_OpenAFile(wmaxpath);
			MConvertStrDel(wmaxpath);
		}
		else if (MFM_FileExistA(("%SystemRoot%\\" + cmds[0] + ".exe").c_str())) {
			LPWSTR wmaxpath = MConvertLPCSTRToLPWSTR((LPCSTR)("%SystemRoot%\\" + cmds[0] + ".exe").c_str());
			MFM_OpenAFile(wmaxpath);
			MConvertStrDel(wmaxpath);
		}
		else printf("Unknow cmd : %s\n", cmds[0].c_str());
	}
	return false;
}
