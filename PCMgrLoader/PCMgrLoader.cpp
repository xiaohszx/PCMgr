//PCMgrLoader.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "PCMgrLoader.h"
#include "..\TaskMgrCore\mapphlp.h"
#include "..\TaskMgrCore\PathHelper.h"

int main();
int main_entry() {
	int rs = main();
	MAppMainExit(rs);
	return rs;
}
int main() {
	if (!MAppMainCanRun()) return -1;
	MAppMainRun();
	return MAppMainGetExitCode();
}


