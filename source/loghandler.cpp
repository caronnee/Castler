#include "loghandler.h"
#include <qaction>
#include <QContextMenuEvent>

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

QString levels[MLevels] = { "Info:", "Warning", "Error" };

#define MAX_MESSAGE_LEN 1024

#include <QScrollBar>

void LogHandler::Report(MessageLevel level, const QString & str)
{
	insertPlainText(levels[level]);
	insertPlainText(" : ");
	insertPlainText(str);
	insertPlainText("\n");
	QScrollBar *sb = verticalScrollBar();
	sb->setValue(sb->maximum());
	update();
}
#include <QMenu>

void LogHandler::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(_clearAction);
	menu.exec(event->globalPos());
	event->accept();
}
