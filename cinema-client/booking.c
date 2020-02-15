#include "booking.h"
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#include "asprintf.h"

#define MAX_SIZE 32

LPTSTR GetFilename(LPCTSTR lpParam) {
	LPTSTR lpBuffer = NULL;
	LPTSTR lpTmp = NULL;
	DWORD len = 0;

	if (lpParam == NULL) {
		return NULL;
	}
	if (!(len = GetEnvironmentVariable(TEXT("APPDATA"), lpBuffer, 0))) {
		return NULL;
	}
	len++;
	if ((lpBuffer = malloc(sizeof(TCHAR) * len)) == NULL) {
		return NULL;
	}
	if (!GetEnvironmentVariable(TEXT("APPDATA"), lpBuffer, len)) {
		free(lpBuffer);
		return NULL;
	}
	lpTmp = lpBuffer;
	if (asprintf(&lpBuffer, TEXT("%s\\%s"), lpBuffer, lpParam) == -1) {
		free(lpBuffer);
		return NULL;
	}
	free(lpTmp);
	if (!CreateDirectory(lpBuffer, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		free(lpBuffer);
		return NULL;
	}
	lpTmp = lpBuffer;
	if (asprintf(&lpBuffer, TEXT("%s\\%s"), lpBuffer, TEXT("sav.dat")) == -1) {
		free(lpBuffer);
		return NULL;
	}
	free(lpTmp);
	return lpBuffer;
}

HBOOKING CreateBooking(LPCTSTR lpParam) {
	HBOOKING hBooking = NULL;
	LPTSTR filename = NULL;

	if ((filename = GetFilename(lpParam)) == NULL) {
		return hBooking;
	}
	hBooking = CreateFile(
		filename,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_HIDDEN,
		NULL
	);
	if (hBooking == INVALID_HANDLE_VALUE) {
		hBooking = NULL;
	}
	free(filename);
	return hBooking;
}

BOOL SetBooking(HBOOKING hBooking, LPCTSTR lpParam) {
	DWORD dwBytesWritten = 0;
	SetFilePointer(hBooking, 0, NULL, FILE_BEGIN);
	SetEndOfFile(hBooking);
	return WriteFile(hBooking, lpParam, _tcslen(lpParam) * sizeof(TCHAR), &dwBytesWritten, NULL);
}

LPTSTR GetBooking(HBOOKING hBooking) {
	LPTSTR lpBuffer = NULL;
	DWORD dwBytesRead = 0;
	DWORD dwBytesReadable = GetFileSize(hBooking, NULL);
	if (dwBytesReadable == INVALID_FILE_SIZE || dwBytesReadable > MAX_SIZE) {
		return NULL;
	}
	if ((lpBuffer = malloc(sizeof(TCHAR) * (dwBytesReadable + 1))) == NULL) {
		return NULL;
	}
	memset(lpBuffer, 0, sizeof(TCHAR) * (dwBytesReadable + 1));
	SetFilePointer(hBooking, 0, NULL, FILE_BEGIN);
	if (!ReadFile(hBooking, lpBuffer, dwBytesReadable, &dwBytesRead, NULL)) {
		return NULL;
	}
	return lpBuffer;
}
