#pragma once

#if _DEBUG
#define DoAssert(cond) if (!(cond)){ __debugbreak();}
#else
#include "loghandler.h"
#define DoAssert(cond) if (!(cond)){ gl_report( LogHanler::MError,"Assertion failed");}
#endif