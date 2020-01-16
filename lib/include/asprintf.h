#pragma once

#include <stdarg.h>

int vasprintf(char **str, const char *format, va_list ap);

int asprintf(char **str, const char *format, ...);
