#include "loghandler.h"

LogHandler::LogHandler(QWidget * parent) : QTextBrowser(parent)
{

}

LogHandler::~LogHandler()
{

}

QString levels[LogHandler::MLevels] = { "Warning:", "Error", "Info" };

void LogHandler::Log(MessageLevel level, const QString & str)
{
	insertPlainText(levels[level]);
	insertPlainText(str);
}
