#include "ReportFunctions.h"

IReportFunction * GHandler;

void SetLogging(IReportFunction * handler)
{
	GHandler = handler;
}

void gf_report(MessageLevel level, const char * format, ...)
{
	char message[256];
	va_list args;
	va_start(args, format);
	vsprintf(message, format, args);
	GHandler->Report(level, message);
	va_end(args);
}