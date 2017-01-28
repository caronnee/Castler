#include "creator.h"
#include "renderer.h"
#include <QtWidgets>
#include "loghandler.h"
#include "imageprovider.h"
#include "opencv2/opencv.hpp"

Creator::Creator(QWidget *parent)
    : QWidget(parent)
{
	_modifier = 0;
	_cameraWindow = new CameraParametersShow(this);
	//_cameraWindow->hide();
	qRegisterMetaType<MessageLevel>("MessageLevel");
	qRegisterMetaType<cv::Mat>("cv::Mat");
	qRegisterMetaType<CalibrationSet>("CalibrationSet");
	qRegisterMetaType<QMatrix4x4>("QMatrix4x4");
	qRegisterMetaType<PositionDesc>("PositionDesc");

    ui.setupUi(this);

	// populate values to comboboxes
	ui.modifierCombo->addItem("None", QVariant(NoneModifier));
	ui.modifierCombo->addItem("Canny", QVariant(CannyModifier));

	// connections
    
	// Global application
	// shortcuts
	_shortcuts.push_back(new QShortcut(QKeySequence("Ctrl+s"), this));
	_shortcuts.push_back(new QShortcut(QKeySequence("Alt+a"), this));
	_shortcuts.push_back(new QShortcut(QKeySequence("Alt+n"), this));
	connect(_shortcuts[0], SIGNAL(activated()), this, SLOT(SaveSettings()));
	connect(_shortcuts[1], SIGNAL(activated()), this, SLOT(AddPoint()));
	connect(_shortcuts[2], SIGNAL(activated()), this, SLOT(AddNewPoint()));

	// worker connects
	connect(this, SIGNAL(modeChangedVisualSignal(int)), _capturer.GetWorker(), SLOT(ChangeVisualMode(int)));
	connect(this, SIGNAL(modeChangedActionSignal(int)), _capturer.GetWorker(), SLOT(ChangeActionMode(int)));
	connect(ui.renderer, SIGNAL( DescChangedSignal(PositionDesc&)), this, SLOT(FillActive(PositionDesc&)));

	// rendered connects
	connect(ui.renderGroup, SIGNAL(buttonClicked(int)), ui.renderer, SLOT(ChangeRenderStyle(int)));
	ui.renderGroup->setId(ui.renderPointsButton, RenderPoints);
	ui.renderGroup->setId(ui.renderWireframeButton, RenderWireframe);
	ui.renderGroup->setId(ui.renderFullButton, RenderComplete);

	connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(LoadModel()));
	connect(ui.renderer, SIGNAL(reportSignal(MessageLevel, const QString &)), ui.infobox, SLOT(Report(MessageLevel, const QString&)));
	connect(ui.reloadShadersButton, SIGNAL(clicked()), ui.renderer, SLOT(ChangeShaders()));
	connect(ui.lockGroup, SIGNAL(buttonClicked(int)), ui.renderer, SLOT(ChangeActiveKeyPos(int)));
	ui.lockGroup->setId(ui.cameraRadioButton, PositionCamera);
	ui.lockGroup->setId(ui.modelRadioButton, PositionModel);
	ui.lockGroup->setId(ui.lightRadioButton, PositionLight);

	// videorender connections
	connect(ui.applyDescButton, SIGNAL(clicked()), this, SLOT(ChangeActiveDesc()));
	connect(ui.saveSettingsButton, SIGNAL(clicked(void)), this, SLOT(SaveSettings()));
	connect(ui.playButton, SIGNAL(clicked(void)), this, SLOT(PlayVideo()));
	connect(ui.pauseButton, SIGNAL(clicked(void)), this, SLOT(Pause(void)));
	connect(ui.cloudPoints, SIGNAL(Finished(void)), this, SLOT(EnablePlay()));
	connect(ui.cloudPoints, SIGNAL(reportSignal(MessageLevel, const QString &)), ui.infobox, SLOT(Report(MessageLevel, const QString&)));
	connect(ui.createMeshButton, SIGNAL(clicked()),this, SLOT(StartCreating()));
	connect(ui.nextFrameButton, SIGNAL(clicked(void)), _capturer.GetWorker(), SLOT(RequestNextFrame()));
	//connect(ui.prevFrameButton, SIGNAL(clicked(void)), ui.cloudPoints, SLOT(RequestPrevFrame()));
	//ui.nextFrameButton->setDisabled(true);
	ui.prevFrameButton->setDisabled(true);
	connect(ui.stopButton, SIGNAL(clicked(void)), this, SLOT(Stop(void)));
	connect(ui.featuresCheckbox, SIGNAL(clicked(void)), this, SLOT(FeaturesFromFrame()));
	connect(ui.stopButton, SIGNAL(clicked(void)), this, SLOT(Stop()));
	connect(ui.greyCheckBox, SIGNAL(clicked(void)), this, SLOT(ShowGreyFrame(void)));
	connect(ui.newButton, SIGNAL(clicked(void)), this, SLOT(CreateNew(void)));

	//calibration connects
	connect(ui.loadCalibrationButton, SIGNAL(clicked()), this, SLOT(LoadCalibration(void)));
	connect(ui.runCalibrationButton, SIGNAL(clicked()), this, SLOT(RunCalibration(void)));
	connect(ui.calibrationFolderButton, SIGNAL(clicked()), this, SLOT(LoadCalibrationImages()));
	connect(_cameraWindow, SIGNAL(calibrationChanged(CalibrationSet)), this, SLOT(SendParameters(CalibrationSet)));
	connect(ui.playUndistortedButton, SIGNAL(clicked()), this, SLOT(ShowUndistorted()));
	connect(ui.saveCalibrationButton, SIGNAL(clicked()), this, SLOT(SaveCalibration()));

	connect(&_capturer, SIGNAL(imageReadySignal(cv::Mat)), ui.cloudPoints, SLOT(setImage(cv::Mat)));

	connect(&_capturer, SIGNAL(reportSignal(MessageLevel, const QString &)), ui.cloudPoints, SLOT(Report(MessageLevel, const QString &)));
	connect(_capturer.GetWorker(), SIGNAL(camParametersSignal(cv::Mat, cv::Mat)), ui.cloudPoints, SLOT(ShowParameters(cv::Mat, cv::Mat)));
	connect(ui.cloudPoints, SIGNAL(setCalibrationSignal(CalibrationSet)), _capturer.GetWorker(), SLOT(ChangeCalibration(CalibrationSet)));

	// comparer connects
	connect(ui.compareNext, SIGNAL(clicked()), this, SLOT(GetNextImagePair()));
	connect(this, SIGNAL(PreparePairSignal(int, int)), _capturer.GetWorker(), SLOT(PreparePair(int,int)));
	connect(_capturer.GetWorker(), SIGNAL(imagePairSignal(cv::Mat, cv::Mat)), this, SLOT(SetCompare(cv::Mat, cv::Mat)));
	connect(ui.applyModifierButton, SIGNAL(clicked()), this, SLOT(SetModifier()));
	//connect(ui.comparePrev, SIGNAL(clicked()), this, SLOT(GetPrevImagePair()));

	// rest of the initialization
	LoadSettings();
}

void Creator::FillActive(PositionDesc & des)
{
	ui.azimuthValue->setValue(des._azimuth);
	ui.elevationValue->setValue(des._elevation);
	ui.xValue->setValue(des._xPos);
	ui.yValue->setValue(des._yPos);
	ui.zValue->setValue(des._zPos);

}

void Creator::ChangeActiveDesc()
{
	PositionDesc desc;
	desc._azimuth = ui.azimuthValue->value();
	desc._elevation = ui.elevationValue->value();
	desc._xPos = ui.xValue->value();
	desc._yPos = ui.yValue->value();
	desc._zPos = ui.zValue->value();
	ui.renderer->ApplyDesc(desc);
}

void Creator::SetModifier()
{
	_modifier = ui.modifierCombo->currentData(Qt::UserRole).toInt();
	emit PreparePairSignal(-2, _modifier);
}

void Creator::SetCompare(cv::Mat left, cv::Mat right)
{
	ui.leftImage->SetImage(left);
	ui.rightImage->SetImage(right);
}

void Creator::GetNextImagePair()
{
	emit PreparePairSignal(0, _modifier);
}

void Creator::FeaturesFromFrame()
{
	emit modeChangedVisualSignal(VisualModeFeatures);
}

void Creator::CreateNew()
{
	ui.inputList->clear();
}

void Creator::ShowGreyFrame()
{
	emit modeChangedVisualSignal(VisualModeGrey);
}

void Creator::Stop()
{
	emit modeChangedVisualSignal(VisualModeStop);
}

void Creator::Pause()
{
	emit modeChangedVisualSignal(VisualModePlay);
}

void Creator::SendParameters( CalibrationSet calibration )
{
	ui.cloudPoints->SetParameters( calibration );
}

void Creator::ShowUndistorted()
{
	_cameraWindow->PrepareCalibration();
	emit modeChangedVisualSignal(VisualModeUndistort);
}


void Creator::RunCalibration()
{
	emit modeChangedActionSignal(ActionModeCalibrate);
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
		_cameraWindow->Load(calSettings);
		
	}
	calSettings.sync();
}

void Creator::LoadCalibration(const QString & input)
{
	InputList::_lastDirectory = input;
	ui.calibrationLabel->setText(input);
	ui.compareLabel->setText("");
	std::string title = FindTitle(input.toStdString().c_str());
	QString calSettingsFile = QApplication::applicationDirPath() + title.c_str() + CCalibrationExt;
	_capturer.Load(input);
	QSettings calSettings(calSettingsFile, QSettings::IniFormat);
	if (calSettings.status() == QSettings::NoError)
	{
		_cameraWindow->Load(calSettings);
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

#include "Filename.h"

void Creator::StartCreating()
{
	// create mesh from all inputs
	int size = ui.inputList->count();
	std::string compl;
	for (int i = 0; i < size; i++)
	{
		auto input = ui.inputList->item(i);
		std::string str = GetFullPath(input->data(Qt::UserRole).toString().toStdString().c_str());
		compl += str + ";";
	}
	_capturer.Load(compl.c_str());
	emit modeChangedActionSignal(ActionModeCreate);
}

void Creator::PlayVideo()
{
	// there is always something selected;set
	// if nothing is selected, return
	QString actualString = ui.inputList->selectedItems()[0]->data(Qt::UserRole).toString();
	_capturer.Load(actualString);
	emit modeChangedActionSignal(ActionModePlay);
}

const char * CSettingFileName = "/settings.castler";

// SETTINGSTRINGS

const char * calibrationInputStr = "calibrationInput";
const char * lastDirStr = "lastDir";
const char * screenStr = "screen";
const char * lastOpenedStr = "lastOpened";
const char * modelsStr = "models";
const char * singleModelStr = "model";
const char * renderModeStr = "renderMode";
const char * lockModeStr = "lockMode";


void Creator::LoadSettings()
{
	QString settingspath = QApplication::applicationDirPath() + CSettingFileName;
	QSettings settings(settingspath, QSettings::IniFormat);

	QVariant str = settings.value(calibrationInputStr);
	ui.calibrationLabel->setText(str.toString());
	if (!str.isNull())
	{
		LoadCalibration(str.toString());		
	}
	str = settings.value(lastDirStr).toString();
	InputList::_lastDirectory = str.toString();

	str = settings.value(screenStr);
	ui.creatorTabs->setCurrentIndex(str.toInt());
	QString last = settings.value(lastOpenedStr).toString();
	QString settingLastName = QApplication::applicationDirPath() + "\\" + last;
	QSettings settingsLast(settingLastName, QSettings::IniFormat);
	int size = settingsLast.beginReadArray(modelsStr);
	for (int i = 0; i<size ; i++)
	{
		settingsLast.setArrayIndex(i);
		ui.inputList->AddInputItem(settingsLast.value(singleModelStr).toString());
	}
	settingsLast.endArray();
	QVariant s = settings.value(renderModeStr);
	int selectedButton = s.toInt();
	auto buttons = ui.renderGroup->buttons();
	for (int i = 0; i < buttons.size(); i++)
	{
		if (ui.renderGroup->id(buttons[i]) == selectedButton)
		{
			buttons[i]->setChecked(true);
			ui.renderer->ChangeRenderStyle(selectedButton);
			break;
		}
	}
	selectedButton = settings.value(lockModeStr).toInt();
	buttons = ui.lockGroup->buttons();
	for (int i = 0; i < buttons.size(); i++)
	{
		if (ui.lockGroup->id(buttons[i]) == selectedButton)
		{
			buttons[i]->setChecked(true);
			ui.renderer->ChangeActiveKeyPos(selectedButton);
			break;
		}
	}
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

void Creator::AddNewPoint()
{
	if (ui.creatorTabs->currentIndex() != ui.creatorTabs->indexOf(ui.compareTab))
	{
		return;
	}
	int size = ui.selectedPoints->topLevelItemCount();
	// just add pair
	// if some point was selected, add it to this point
	QStringList list;
	QString str = QString::asprintf("%d", size);
	list.append(str);
	QTreeWidgetItem * item = new QTreeWidgetItem(list, 0);
	ui.selectedPoints->addTopLevelItem(item);
	ui.selectedPoints->clearSelection();
	ui.selectedPoints->setItemSelected(item, true);
}


void Creator::AddPoint()
{
	QStringList list;
	QString str;
	QTreeWidgetItem * item = ui.selectedPoints->selectedItems()[0];
	list.append(QString::asprintf("%d", item->childCount()));
	QTreeWidgetItem * child = new QTreeWidgetItem(list, 0);
	item->addChild(child);
}

void Creator::SaveSettings()
{
	QString settingspath = QApplication::applicationDirPath() + CSettingFileName;
	QSettings settings(settingspath, QSettings::IniFormat);

	settings.setValue(screenStr, ui.creatorTabs->currentIndex());
	if (ui.calibrationLabel->text().isEmpty() == false)
	{
		settings.setValue(calibrationInputStr, ui.calibrationLabel->text());
	}
	settings.setValue(renderModeStr, ui.renderGroup->checkedId());
	settings.setValue(lockModeStr, ui.lockGroup->checkedId());
	settings.setValue(lastDirStr, InputList::_lastDirectory);
	settings.setValue(lastOpenedStr, "last");
	settings.setValue(singleModelStr, ui.modelName->text());

	QString name = "last";
	if (ui.creatorLabel->text().size())
	{
		name = ui.creatorLabel->text();
	}
	QString model = QApplication::applicationDirPath() + "\\" + name;
	QSettings modelSettings(model, QSettings::IniFormat);
	modelSettings.beginWriteArray(modelsStr);
	
	for (int i = 0; i < ui.inputList->count(); i++)
	{
		modelSettings.setArrayIndex(i);
		QVariant str = ui.inputList->item(i)->data(Qt::UserRole);
		modelSettings.setValue(singleModelStr, str);
	}
	modelSettings.endArray();

	modelSettings.sync();
	settings.sync();
}
