#pragma once

#include <stdarg.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __unix__
#define LPTSTR char*
#define LPCTSTR const char*
#endif

int asprintf(LPTSTR* str, LPCTSTR format, ...);
