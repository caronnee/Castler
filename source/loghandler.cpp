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

void gf_report(LogHandler::MessageLevel level, const char * message)
{
	GHandler->Report(level, message);
}

void LogHandler::Report(MessageLevel level, const QString & str)
{
	insertPlainText(levels[level]);
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
