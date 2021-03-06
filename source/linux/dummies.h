#pragma once

#include <stdarg.h>

#define HD_LED 1

void DeleteCriticalSection(CRITICAL_SECTION * criticalSection);
void InitializeCriticalSection(CRITICAL_SECTION * criticalSection);
void EnterCriticalSection(CRITICAL_SECTION * criticalSection);
void LeaveCriticalSection(CRITICAL_SECTION * criticalSection);
void OutputDebugString(const char * str);

extern int g_nAltCharSetOffset; // alternate character set

HWND GetDesktopWindow();
void ExitProcess(int);
