#ifndef CREATOR_H
#define CREATOR_H

#include "QtWidgets/QWidget.h"
#include "ui_creator.h"
#include "opencv2/highgui/highgui.hpp"
#include <QShortcut>
#include "FrameProcessor.h"

class Creator : public QWidget, public IReportFunction
{
	Q_OBJECT
		;

	// application shortcuts
	std::vector<QShortcut *> _shortcuts;

	// one common processor for images
	FrameProcessor _capturer;

	int _modifier;

public:
    Creator(QWidget *parent = 0);

	void Report(MessageLevel level, const QString & str);
	void LoadCalibration(const QString & str);

	~Creator();

signals:
	void modeChangedSignal(int);
	void PreparePairSignal(int,int);

private slots:
	void FillActive(PositionDesc & des);
	void ChangeActiveDesc();
	void SetModifier();
	void GetNextImagePair();
	void SetCompare(cv::Mat left, cv::Mat right);
	void FeaturesFromFrame();
	void CreateNew();
	void ShowGreyFrame();
	void Stop();
	void Pause();
	void SendParameters();
	void SaveCalibration();
	void EnablePlay();
	void StartCreating();
	void PlayVideo();
	void LoadSettings();
	void LoadModel();
	void AddNewPoint();
	void AddPoint();
	void SaveSettings();
	void LoadCalibration();
	void RunCalibration();
	void SetCalibCamera(cv::Mat camera, int type);
	void ShowUndistorted();
	void LoadCalibrationImages();
private:
    Ui::CreatorContent ui;
};

#endif // CREATOR_H
