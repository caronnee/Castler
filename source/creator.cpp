#include "creator.h"
#include "renderer.h"
#include <QtWidgets>
#include "loghandler.h"

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	for (int c = 0; c < ui.cameraMatrix->horizontalHeader()->count(); ++c)
	{
		ui.cameraMatrix->horizontalHeader()->setSectionResizeMode(
			c, QHeaderView::Stretch);
	}

	for (int c = 0; c < ui.cameraMatrix->verticalHeader()->count(); ++c)
	{
		ui.cameraMatrix->verticalHeader()->setSectionResizeMode(
			c, QHeaderView::Stretch);
	}

	qRegisterMetaType<MessageLevel>("MessageLevel");
	qRegisterMetaType<cv::Mat>("cv::Mat");
	// connections
    
	// Global application
	// shortcuts
	_shortcuts.push_back( new QShortcut(QKeySequence("Ctrl+s"),this));
	connect(_shortcuts[0], SIGNAL(activated()), this, SLOT(SaveSettings()));

	// Data connections
	connect(ui.saveSettingsButton, SIGNAL(clicked(void)), this, SLOT(SaveSettings()));
	connect(ui.playButton, SIGNAL(clicked(void)), this, SLOT(PlayVideo()));
	connect(ui.pauseButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(Pause(void)));
	connect(ui.cloudPoints, SIGNAL(Finished(void)), this, SLOT(EnablePlay()));
	connect(ui.cloudPoints, SIGNAL(reportSignal(MessageLevel, const QString &)), ui.infobox, SLOT(Report(MessageLevel, const QString&)));
	connect(ui.nextFrameButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(RequestNextFrame()));
	connect(ui.prevFrameButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(RequestPrevFrame()));
	connect(ui.stopButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(Stop(void)));
	connect(ui.featuresButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(FeaturesFromFrame()));
	connect(ui.stopButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(Stop()));
	connect(ui.greyButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(ShowGreyFrame(void)));

	//calibration connects
	connect(ui.loadCalibrationButton, SIGNAL(clicked()), this, SLOT(LoadCalibration(void)));
	connect(ui.runCalibrationButton, SIGNAL(clicked()), this, SLOT(RunCalibration(void)));
	connect(ui.showCalibrationGrey, SIGNAL(clicked()), ui.calibrationVideo, SLOT(ShowGreyFrame(void)));
	connect(ui.calibrationFolderButton, SIGNAL(clicked()), this, SLOT(LoadCalibrationImages()));
	connect(ui.calibrationVideo, SIGNAL(reportSignal(MessageLevel, const QString & )), ui.infobox, SLOT(Report(MessageLevel,const QString&)));
	connect(ui.stopCalibrationButton, SIGNAL(clicked()), ui.calibrationVideo, SLOT(Stop()));
	connect(ui.calibrationVideo, SIGNAL(setCameraSignal(cv::Mat,int)), this, SLOT(SetCalibCamera(cv::Mat,int)));
	connect(ui.applyCalibration, SIGNAL(clicked()), this, SIGNAL(setParameters()));

	// rest of the initialization
	LoadSettings();
}

void Creator::SetCalibCamera(cv::Mat camera, int type)
{
	if (type == 0)
	{
#define CAM_X 3
#define CAM_Y 3
		DoAssert(camera.cols == 3);
		DoAssert(camera.rows == 3);
		for (int i = 0; i < CAM_X; i++)
		{
			for (int j = 0; j < CAM_Y; j++)
			{
				float v = camera.at<double>(i, j);
				QVariant val(v);
				ui.cameraMatrix->setItem(i, j, new QTableWidgetItem(val.toString()));
			}
		}
		// set focus
		double focusVal = camera.at<double>(0, 0);
		ui.focusValue->setValue(focusVal);
	}
	else if ( type == 1)
	{
		std::vector<double> d(camera);
		// set distortion parameters
		ui.k1->setValue(camera.at<double>(0, 0));
		ui.k2->setValue(camera.at<double>(1, 0));
		ui.k3->setValue(camera.at<double>(4.0));

		// tangential will be zero - we don't care for them
	}
	else
	{
		DoAssert(false);
	}
}
void Creator::RunCalibration()
{
	ui.calibrationVideo->Start(ui.calibrationLabel->text(), VideoRenderer::ActionCalibrate);
}

void Creator::LoadCalibrationImages()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	InputList::_lastDirectory = str;
	ui.calibrationLabel->setText(str);
}
void Creator::LoadCalibration()
{
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		"Select one or more video files to open",
		InputList::_lastDirectory,
		"Images (*.avi *.mpeg *.mp4 *.3gp *.calibrated)");
	if (files.count() == 0)
		return;
	InputList::_lastDirectory = files[0];
	ui.calibrationLabel->setText(files[0]);
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
	ui.cloudPoints->Start(defname, VideoRenderer::ActionDetect);
}

const char * CSettingFileName = "/settings.castler";

void Creator::LoadSettings()
{
	QString settingspath = QApplication::applicationDirPath() + CSettingFileName;
	QSettings settings(settingspath, QSettings::IniFormat);

	QVariant str = settings.value("calibrationInput");
	ui.calibrationLabel->setText(str.toString());
	str = settings.value("lastDir").toString();
	InputList::_lastDirectory = str.toString();

	QString last = settings.value("lastOpened").toString();
	QString settingLastName = QApplication::applicationDirPath() + "\\" + last;
	QSettings settingsLast(settingLastName, QSettings::IniFormat);
	int size = settingsLast.beginReadArray("input");
	for (int i = 0; i<size ; i++)
	{
		settingsLast.setArrayIndex(i);
		ui.inputList->addVideo(settingsLast.value("input").toString());
	}
	settingsLast.endArray();
}

void Creator::SaveSettings()
{
	QString settingspath = QApplication::applicationDirPath() + CSettingFileName;
	QSettings settings(settingspath, QSettings::IniFormat);

	settings.setValue("calibrationInput", ui.calibrationLabel->text());
	settings.setValue("lastDir", InputList::_lastDirectory);
	settings.setValue("lastOpened", "last");

	QString name = "last";
	if (ui.modelLabel->text().size())
	{
		name = ui.modelLabel->text();
	}
	QString model = QApplication::applicationDirPath() + "\\" + name;
	QSettings modelSettings(model, QSettings::IniFormat);
	modelSettings.beginWriteArray("input");
	
	for (int i = 0; i < ui.inputList->count(); i++)
	{
		modelSettings.setArrayIndex(i);
		QVariant str = ui.inputList->item(i)->data(Qt::UserRole);
		modelSettings.setValue("input", str);
	}
	modelSettings.endArray();

	
	modelSettings.sync();
	settings.sync();
	//InputList * list = ui.inputList;
	//QFile file("settings.cfg");
	//if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	//{
	//	ui.infobox->Report(MError,"Unable to open setting file");
	//	return;
	//}
	//list->Save(file);
}
