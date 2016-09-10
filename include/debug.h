#pragma once

#include "ReportFunctions.h"

#if _DEBUG
#define DoAssert(cond) if (!(cond)){ __debugbreak();}
#else
#include <windows.h>
#define DoAssert(cond) if (!(cond)){ OutputDebugStringA( "Assertion failed");}
#endif