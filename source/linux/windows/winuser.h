#pragma once

#include "linux/windows/handles.h"

typedef void * HCURSOR;

#define IDC_WAIT "IDC_WAIT"

HCURSOR LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName);
HCURSOR SetCursor(HCURSOR hCursor);
