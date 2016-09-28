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