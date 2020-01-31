#include "asprintf.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#endif

#ifdef __unix__
#define TCHAR char
#define LPTSTR char*
#define LPCTSTR const char*
#define _vsctprintf(Format, ArgList) vsnprintf(NULL, 0, Format, ArgList)
#define _vstprintf_s(Buffer, MaxCount, Format, ArgList) vsprintf(Buffer, Format, ArgList)
#endif

int vasprintf(LPTSTR* str, LPCTSTR format, va_list args){
	size_t size;
	va_list tmp;
	va_copy(tmp, args);
	size = _vsctprintf(format, tmp);
	va_end(tmp);
	if (size == -1){
		return -1;
	}
	if ((*str = (LPTSTR)malloc(sizeof(TCHAR) * (size + 1))) == NULL){
		return -1;
	}
	size = _vstprintf_s(*str, size + 1, format, args);
	return size;
}

int asprintf(LPTSTR* str, LPCTSTR format, ...){
	size_t size;
	va_list args;
	va_start(args, format);
	size = vasprintf(str, format, args);
	va_end(args);
	return size;
}