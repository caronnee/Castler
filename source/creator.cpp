#include "creator.h"
#include "renderer.h"
#include <QtWidgets>
#include "loghandler.h"

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

/// connections

	// connections
    //connect(ui.loadButton, SIGNAL(clicked(void)), this, SLOT(reload()));
	connect(ui.saveSettingsButton, SIGNAL(clicked(void)), this, SLOT(SaveSettings()));
	connect(ui.playButton, SIGNAL(clicked(void)), this, SLOT(PlayVideo()));
	connect(ui.pauseButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(Pause(void)));
	connect(ui.cloudPoints, SIGNAL(Finished(void)), this, SLOT(EnablePlay()));
	connect(ui.nextFrameButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(RequestNextFrame()));
	connect(ui.prevFrameButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(RequestPrevFrame()));
	connect(ui.stopButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(Stop(void)));
	connect(ui.featuresButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(FeaturesFromFrame()));
	connect(ui.stopButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(Stop));
	connect(ui.greyButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(ShowGreyFrame(void)));

	QFile file("settings.cfg");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		ui.inputList->Load(file);
	}
	// TODO change for this ;)
	SetLogging(ui.infobox);
}

Creator::~Creator()
{
	
}

#include "opencv2/opencv.hpp"


void Creator::EnablePlay()
{
	ui.playButton->setDisabled(false);
}

void Creator::PlayVideo()
{
	QString defname = ui.inputList->selectedItems()[0]->data(Qt::UserRole).toString();
	if (defname.size() == 0)
		return;
	ui.cloudPoints->Load(defname);
}

void Creator::SaveSettings()
{
	InputList * list = ui.inputList;
	QFile file("settings.cfg");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		ui.infobox->Report(LogHandler::MError,"Unable to open setting file");
		return;
	}
	list->Save(file);
}
