#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

//	Variabili globali:

HANDLE hSavefile;		// Handle al file di salvataggio

extern void getPath(LPTSTR*, LPCTSTR, LPCTSTR);

extern void getSaveFileName(LPTSTR*, LPCTSTR);

extern HANDLE getSavefile(LPCTSTR);

extern BOOL InitSavefile(LPCTSTR);

extern BOOL store(LPCTSTR);

extern BOOL load(LPTSTR);
