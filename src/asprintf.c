#include "asprintf.h"

#include <stdio.h>
#include <stdlib.h>

int vasprintf(char **str, const char *format, va_list ap){
	size_t size;
	va_list tmp;
	va_copy(tmp, ap);
	size = vsnprintf(NULL, 0, format, tmp);
	va_end(tmp);
	if (size == -1){
		return -1;
	}
	*str = (char *)malloc(sizeof(char) * (size + 1));
	if (str == NULL){
		return -1;
	}
	size = vsprintf(*str, format, ap);
	return size;
}

int asprintf(char **str, const char *format, ...){
	size_t size;
	va_list ap;
	va_start(ap, format);
	size = vasprintf(str, format, ap);
	va_end(ap);
	return size;
}
