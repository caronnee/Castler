#include "loghandler.h"
#include <qaction>
#include <QContextMenuEvent>

LogHandler * GHandler = NULL;

void SetLogging(LogHandler * handler)
{
	GHandler = handler;
}

void LogHandler::clearLog()
{
	this->clear();
}
LogHandler::LogHandler(QWidget * parent) : QTextBrowser(parent)
{
	_clearAction = new QAction("Clear log",this);
	connect(_clearAction, SIGNAL(triggered(void)), this, SLOT(clearLog(void)));
}

LogHandler::~LogHandler()
{

}

QString levels[LogHandler::MLevels] = { "Warning:", "Error", "Info" };

#define MAX_MESSAGE_LEN 1024

void gf_report(LogHandler::MessageLevel level, const char * format, ... )
{
	char message[256];
	va_list args;
	va_start(args, format);
	vsprintf(message, format, args);
	GHandler->Report(level, message);
	va_end(args);
}

void LogHandler::Report(MessageLevel level, const QString & str)
{
	insertPlainText(levels[level]);
	insertPlainText(" : ");
	insertPlainText(str);
	insertPlainText("\n");
}
#include <QMenu>

void LogHandler::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(_clearAction);
	menu.exec(event->globalPos());
	event->accept();
}
