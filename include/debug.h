#pragma once

#include "ReportFunctions.h"

#if _DEBUG
#define DoAssert(cond) if (!(cond)){ __debugbreak();}
#else
#define DoAssert(cond) if (!(cond)){ OutputDebugString( "Assertion failed");}
#endif