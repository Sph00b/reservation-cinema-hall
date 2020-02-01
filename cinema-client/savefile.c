#include "savefile.h"
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

#include "asprintf.h"

#define MAX_SIZE 32

int MyGetEnvironmentVariable(LPCTSTR lpName, LPTSTR* lpBuffer) {
	DWORD len = 0;
	if (lpBuffer == NULL) {
		return 1;
	}
	if (!(len = GetEnvironmentVariable(lpName, *lpBuffer, len))) {
		return 1;
	}
	if ((*lpBuffer = (LPTSTR) malloc(sizeof(TCHAR) * len)) == NULL) {
		return 1;
	}
	if (!GetEnvironmentVariable(lpName, *lpBuffer, len)) {
		free(*lpBuffer);
		lpBuffer = NULL;
		return 1;
	}
	return 0;
}

int InitSavefile(LPCTSTR lpName) {
	LPTSTR env_var;
	LPTSTR path;
	HANDLE htmp;
	MyGetEnvironmentVariable(TEXT("APPDATA"), &env_var);
	if (asprintf(&path, TEXT("%s\\%s"), env_var, lpName) == -1) {
		free(env_var);
		return 1;
	}
	if (!CreateDirectory(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		free(path);
		free(env_var);
		return 1;
	}
#ifdef _DEBUG
	_tprintf(TEXT("SAVEFILE DIRECTORY READY\n"));
#endif
	asprintf(&path, TEXT("%s\\%s"), path, TEXT("sav.dat"));
	htmp = CreateFile(
		path,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_HIDDEN,
		NULL
	);
	free(path);
	free(env_var);
	if (htmp == INVALID_HANDLE_VALUE) {
		return 1;
	}
#ifdef _DEBUG
	_tprintf(TEXT("SAVEFILE INITIALIZED\n"));
#endif
	hSaveFile = htmp;
	return 0;
}

BOOL savStore(LPCTSTR lpBuffer) {
	DWORD dwBytesWritten = 0;
	SetFilePointer(hSaveFile, 0, NULL, FILE_BEGIN);
	SetEndOfFile(hSaveFile);
	WriteFile(hSaveFile, lpBuffer, _tcslen(lpBuffer) * sizeof(TCHAR), &dwBytesWritten, NULL);
	if (_tcslen(lpBuffer) == dwBytesWritten) {
#ifdef _DEBUG
		_tprintf(TEXT("DATA STORED IN THE SAVEFILE\n"));
#endif
		return TRUE;
	}
	else {
		return FALSE;
	}
}

LPTSTR savLoad() {
	DWORD dwBytesRead = 0;
	DWORD dwBytesReadable = GetFileSize(hSaveFile, NULL);
	if (dwBytesReadable == INVALID_FILE_SIZE || dwBytesReadable > MAX_SIZE) {
		return NULL;
	}
	LPTSTR lpBuffer = (LPTSTR) calloc(dwBytesReadable + 1, sizeof(TCHAR));
	SetFilePointer(hSaveFile, 0, NULL, FILE_BEGIN);
	ReadFile(hSaveFile, lpBuffer, dwBytesReadable, &dwBytesRead, NULL);
	if (dwBytesReadable == dwBytesRead) {
#ifdef _DEBUG
		_tprintf(TEXT("DATA LOADED FROM THE SAVEFILE\n"));
#endif
		return lpBuffer;
	}
	else {
		return NULL;
	}
}
