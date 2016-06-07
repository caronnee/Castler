#include "creator.h"
#include "renderer.h"
#include <QtWidgets>

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

/// connecters

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
    timer->start(20);

	// connections
    connect(ui.loadButton, SIGNAL(clicked(void)), this, SLOT(reload()));
	connect(ui.saveSettingsButton, SIGNAL(clicked(void)), this, SLOT(SaveSettings()));

	QFile file("settings.cfg");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		ui.inputList->Load(file);
	}
}

Creator::~Creator()
{
	
}

void Creator::SaveSettings()
{
	InputList * list = ui.inputList;
	QFile file("settings.cfg");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		ui.infobox->Log(LogHandler::MError,"Unable to open setting file");
		return;
	}
	list->Save(file);
}
void Creator::reload()
{
  
}

void Creator::rotateOneStep()
{
    ui.renderWidget->rotateBy(+2 * 16, +2 * 16, -1 * 16);
}