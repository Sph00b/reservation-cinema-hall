#include "savefile.h"

#define DEBUG

void getPath(LPTSTR* path, LPCTSTR var, LPCTSTR dir) {
	size_t size = GetEnvironmentVariable(var, NULL, 0) + _tcslen(TEXT("\\")) + _tcslen(dir);
	*path = malloc(size);
	GetEnvironmentVariable(var, *path, size);
	_tcscat_s(*path, size, TEXT("\\"));
	_tcscat_s(*path, size, dir);
}

void getSaveFileName(LPTSTR* filename, LPCTSTR path) {
	size_t size = _tcslen(path) + _tcslen(TEXT("\\")) + _tcslen(TEXT("sav.txt")) + sizeof(TCHAR);
	*filename = malloc(size);
	_tcscpy_s(*filename, size, path);
	_tcscat_s(*filename, size, TEXT("\\"));
	_tcscat_s(*filename, size, TEXT("sav.txt"));
}

HANDLE getSavefile(LPCTSTR pname) {
	LPTSTR path;
	getPath(&path, TEXT("APPDATA"), "PrenotazioneCinema");							// pname);
	if (!CreateDirectory(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		_ftprintf(stderr, TEXT("Program directory unreachable, errcode: %d"), GetLastError());
		getch();
		exit(GetLastError());
	}
#ifdef DEBUG
	printf("SAVEFILE DIRECTORY READY\n");
#endif
	LPTSTR filename;
	getSaveFileName(&filename, path);
	HANDLE savefile = CreateFile(
		filename,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_HIDDEN,
		NULL
	);
	if (savefile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, TEXT("Save data can't be load, errcode: %d"), GetLastError());
		getch();
		exit(GetLastError());
	}
#ifdef DEBUG
	printf("SAVEFILE READY\n");
#endif
	return savefile;
}

BOOL InitSavefile(LPCTSTR pname) {
	hSavefile = getSavefile(pname);
#ifdef DEBUG
	printf("SAVEFILE INITIALIZED\n");
#endif
	return TRUE;
}

BOOL store(LPCTSTR lpBuffer) {
	DWORD dwBytesWritten = 0;
	WriteFile(hSavefile, lpBuffer, _tcslen(lpBuffer), &dwBytesWritten, NULL);
	if (_tcslen(lpBuffer) == dwBytesWritten) {
#ifdef DEBUG
		printf("DATA STORED IN THE SAVEFILE\n");
#endif
		return TRUE;
	}
	else {
		return FALSE;
	}
}

BOOL load(LPTSTR lpBuffer) {
	DWORD dwBytesRead = 0;
	WriteFile(hSavefile, lpBuffer, _tcslen(lpBuffer), &dwBytesRead, NULL);
	if (_tcslen(lpBuffer) == dwBytesRead) {
#ifdef DEBUG
		printf("DATA LOADED FROM THE SAVEFILE\n");
#endif
	return TRUE;
}
	else {
	return FALSE;
	}
}
