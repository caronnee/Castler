#include "creator.h"
#include "renderer.h"
#include <QtWidgets>
#include "loghandler.h"
#include "imageprovider.h"
#include "opencv2/opencv.hpp"

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
	qRegisterMetaType<MessageLevel>("MessageLevel");
	qRegisterMetaType<cv::Mat>("cv::Mat");
	qRegisterMetaType<CalibrationSet>("CalibrationSet");

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


	// connections
    
	// Global application
	// shortcuts
	_shortcuts.push_back( new QShortcut(QKeySequence("Ctrl+s"),this));
	connect(_shortcuts[0], SIGNAL(activated()), this, SLOT(SaveSettings()));

	// rendered connects
	connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(LoadModel()));
	connect(ui.renderer, SIGNAL(reportSignal(MessageLevel, const QString &)), ui.infobox, SLOT(Report(MessageLevel, const QString&)));

	// videorender connections
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
	connect(ui.calibrationFolderButton, SIGNAL(clicked()), this, SLOT(LoadCalibrationImages()));
	connect(ui.calibrationVideo, SIGNAL(reportSignal(MessageLevel, const QString & )), ui.infobox, SLOT(Report(MessageLevel,const QString&)));
	connect(ui.stopCalibrationButton, SIGNAL(clicked()), ui.calibrationVideo, SLOT(Stop()));
	connect(ui.calibrationVideo, SIGNAL(setCameraSignal(cv::Mat,int)), this, SLOT(SetCalibCamera(cv::Mat,int)));
	connect(ui.applyCalibrationButton, SIGNAL(clicked()), this, SLOT(SendParameters()));
	connect(ui.playUndistortedButton, SIGNAL(clicked()), this, SLOT(ShowUndistorted()));
	connect(ui.saveCalibrationButton, SIGNAL(clicked()), this, SLOT(SaveCalibration()));
	// rest of the initialization
	LoadSettings();
}

void Creator::SendParameters()
{
	CalibrationSet calibration;
	calibration.px = ui.principalXValue->value();
	calibration.py = ui.principalYValue->value();

	// focus
	calibration.fx = ui.focusValue->value();
	calibration.fy = ui.focusValue->value();

	calibration.k1 = ui.k1->value();
	calibration.k2 = ui.k2->value();
	calibration.k3 = ui.k3->value();

	cv::Mat mat = cv::Mat::eye(3, 3, CV_64F);
	mat.at<double>(0, 0) = ui.cameraMatrix->item(0, 0)->data(Qt::UserRole).toDouble();
	mat.at<double>(0, 1) = ui.cameraMatrix->item(0, 1)->data(Qt::UserRole).toDouble();
	mat.at<double>(0, 2) = ui.cameraMatrix->item(0, 2)->data(Qt::UserRole).toDouble();

	mat.at<double>(1, 0) = ui.cameraMatrix->item(1, 0)->data(Qt::UserRole).toDouble();
	mat.at<double>(1, 1) = ui.cameraMatrix->item(1, 1)->data(Qt::UserRole).toDouble();
	mat.at<double>(1, 2) = ui.cameraMatrix->item(1, 2)->data(Qt::UserRole).toDouble();

	mat.at<double>(2, 0) = ui.cameraMatrix->item(2, 0)->data(Qt::UserRole).toDouble();
	mat.at<double>(2, 1) = ui.cameraMatrix->item(2, 1)->data(Qt::UserRole).toDouble();
	mat.at<double>(2, 2) = ui.cameraMatrix->item(2, 2)->data(Qt::UserRole).toDouble();

	calibration.camera = mat;

	ui.calibrationVideo->SetParameters( calibration );
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

void Creator::ShowUndistorted()
{
	ui.calibrationVideo->Start(ui.calibrationLabel->text(), VideoRenderer::ActionUndistort);
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
	LoadCalibration(str);
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
	LoadCalibration(files[0]);
}

const char * CCalibrationExt = ".calibration";
#include "Filename.h"

void Creator::SaveCalibration()
{
	QString input = ui.calibrationLabel->text();
	std::string title = FindTitle(input.toStdString().c_str());
	QString calSettingsFile = QApplication::applicationDirPath() + title.c_str() + CCalibrationExt;
	QSettings calSettings(calSettingsFile, QSettings::IniFormat);
	if (calSettings.status() == QSettings::NoError)
	{
		calSettings.setValue("k1",ui.k1->value());
		calSettings.setValue("k2",ui.k2->value());
		calSettings.setValue("k3",ui.k3->value());
		calSettings.setValue("f", ui.focusValue->value());
		calSettings.setValue("px",ui.principalXValue->value());
		calSettings.setValue("py",ui.principalYValue->value());
	}
	calSettings.sync();
}

void Creator::LoadCalibration(const QString & input)
{
	InputList::_lastDirectory = input;
	ui.calibrationLabel->setText(input);

	std::string title = FindTitle(input.toStdString().c_str());
	QString calSettingsFile = QApplication::applicationDirPath() + title.c_str() + CCalibrationExt;
	QSettings calSettings(calSettingsFile, QSettings::IniFormat);
	if (calSettings.status() == QSettings::NoError)
	{
		QVariant v;
		v = calSettings.value("k1");
		ui.k1->setValue(v.toDouble());
		v = calSettings.value("k2");
		ui.k2->setValue(v.toDouble());
		v = calSettings.value("k3");
		ui.k3->setValue(v.toDouble());

		v = calSettings.value("f");
		ui.focusValue->setValue(v.toDouble());

		v = calSettings.value("px");
		ui.principalXValue->setValue(v.toDouble());

		v = calSettings.value("py");
		ui.principalYValue->setValue(v.toDouble());
	}
}

Creator::~Creator()
{
	
}

void Creator::Report(MessageLevel level, const QString & str)
{
	ui.infobox->Report(level, str);
}

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
	if (!str.isNull())
	{
		LoadCalibration(str.toString());		
	}
	str = settings.value("lastDir").toString();
	InputList::_lastDirectory = str.toString();

	str = settings.value("screen");
	ui.creatorTabs->setCurrentIndex(str.toInt());
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

void Creator::LoadModel()
{
	// find the ply file
	QString str = QFileDialog::getOpenFileName(this, tr("Open model"),
		InputList::_lastDirectory.toStdString().c_str(),"Model files(*.ply *.xpm *.jpg)");

	if (str.isEmpty())
		return;
	InputList::_lastDirectory = str;
	ui.modelName->setText(str);
	
	//load model
	ui.renderer->Load(str);
}
void Creator::SaveSettings()
{
	QString settingspath = QApplication::applicationDirPath() + CSettingFileName;
	QSettings settings(settingspath, QSettings::IniFormat);

	settings.setValue("screen", ui.creatorTabs->currentIndex());
	settings.setValue("calibrationInput", ui.calibrationLabel->text());
	settings.setValue("lastDir", InputList::_lastDirectory);
	settings.setValue("lastOpened", "last");
	settings.setValue("model", ui.modelName->text());

	QString name = "last";
	if (ui.creatorLabel->text().size())
	{
		name = ui.creatorLabel->text();
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
