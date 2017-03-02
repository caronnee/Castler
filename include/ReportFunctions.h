#pragma once

#include <QString>

#define MESSAGES(XX)\
	XX(Info, "blue")\
	XX(Warning, "orange")\
	XX(Error, "red")\
	XX(Normal, "black")\

#define MESSAGE_ENUM(name,color) M##name,

enum MessageLevel
{
	MESSAGES( MESSAGE_ENUM )
	MLevels
};

class IReportFunction
{
public:
	virtual void Report(MessageLevel level, const QString & str) = 0;
};