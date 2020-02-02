#include "asprintf.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#endif

#ifdef _WIN32
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
	return (int)size;
}
#elif __unix__
int vasprintf(char** str, const char* format, va_list args) {
	int size;
	va_list tmp;
	va_copy(tmp, args);
	size = vsnprintf(NULL, 0, format, tmp);
	va_end(tmp);
	if (size == -1) {
		return -1;
	}
	if ((*str = (char*)malloc(sizeof(char) * (size_t)(size + 1))) == NULL) {
		return -1;
	}
	size = vsprintf(*str, format, args);
	return size;
}
#endif

#ifdef _WIN32
int asprintf(LPTSTR* str, LPCTSTR format, ...){
	size_t size;
	va_list args;
	va_start(args, format);
	size = vasprintf(str, format, args);
	va_end(args);
	return (int)size;
}
#elif __unix__
int asprintf(char** str, const char* format, ...) {
	int size;
	va_list args;
	va_start(args, format);
	size = vasprintf(str, format, args);
	va_end(args);
	return (int)size;
}
#endif