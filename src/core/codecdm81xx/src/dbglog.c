#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dbglog.h"

int gDbgLevel = DBGLVL_SETUP;

void dbg_printf (const char *format, ...)
{
	char buffer[1024 + 1];
	va_list arg;
	va_start (arg, format);
	vsprintf (buffer, format, arg);
	fprintf(stderr, ":%s", buffer);
}

void DumpHex(unsigned char *pData, int nSize)
{
	int i;
	for (i=0; i < nSize; i++)
		DBG_PRINT("%02x ", pData[i]);
	DBG_PRINT("\n");
}
