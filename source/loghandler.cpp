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

#define ERROR_NAME(name, color) _STRINGIZE(name),

const char * levels[MLevels] = { 
	MESSAGES ( ERROR_NAME )
};

#define MAX_MESSAGE_LEN 1024

#include <QScrollBar>

#define MESSAGES_COLORS(name,color) "<font color=\"" color "\">%s </font>",
const char * fontColors[] =
{
	MESSAGES(MESSAGES_COLORS)
};
void LogHandler::Report(MessageLevel level, const QString & str)
{
	QString sStr = QString::asprintf(fontColors[level], levels[level]);
	insertHtml(sStr);
	sStr = QString::asprintf(fontColors[MNormal], str.toStdString().c_str());
	insertHtml(sStr);
	insertPlainText("\n");
	QScrollBar *sb = verticalScrollBar();
	sb->setValue(sb->maximum());
}
#include <QMenu>

void LogHandler::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(_clearAction);
	menu.exec(event->globalPos());
	event->accept();
}
