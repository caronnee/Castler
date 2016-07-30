#pragma once

#include "ReportFunctions.h"

#if _DEBUG
#define DoAssert(cond) if (!(cond)){ __debugbreak();}
#else
#define DoAssert(cond) if (!(cond)){ gf_report( MError,"Assertion failed");}
#endif