#include "base.h"
#include "log.h"

void prn(const char* format, ...)
{
	char str[2048];
	va_list vl;
	va_start(vl, format);
	vsprintf_s(str, format, vl);
	printf(str);
	va_end(vl);
}