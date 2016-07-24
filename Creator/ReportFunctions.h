#pragma once

#include <QString>

enum MessageLevel
{
	MInfo,
	MWarning,
	MError,
	MLevels
};


class IReportFunction
{
public:
	virtual void Report(MessageLevel level, const QString & str) = 0;
};

#define ErrorMessage(format, ...) \
    do { \
		__debugbreak();\
        gf_report( MError, format, ##__VA_ARGS__); \
    } while(0)

#define WarningMessage(format, ...) \
    do { \
        gf_report( MWarning, format, ##__VA_ARGS__); \
    } while(0)

#define InfoMessage(format, ...) \
    do { \
        gf_report( MInfo, format, ##__VA_ARGS__); \
    } while(0)


void SetLogging(IReportFunction * handler);
void gf_report(MessageLevel level, const char * format, ...);