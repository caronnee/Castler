#ifndef CREATOR_H
#define CREATOR_H

#include <QtWidgets/QWidget>
#include "ui_creator.h"
#include "opencv2/highgui/highgui.hpp"
#include <QShortcut>
#include "FrameProcessor.h"

class Creator : public QWidget, public IReportFunction
{
	Q_OBJECT

	
	// application shorcuts
	std::vector<QShortcut *> _shortcuts;

	// one common processor for images
	FrameProcessor _capturer;

public:
    Creator(QWidget *parent = 0);

	void Report(MessageLevel level, const QString & str);
	void LoadCalibration(const QString & str);

	~Creator();

signals:
	void modeChanged(int);
	void PreparePairSignal(int);

private slots:
    void GetNextImagePair();
	void SetCompare(cv::Mat left, cv::Mat right);
	void FeaturesFromFrame();
	void ShowGreyFrame();
	void Stop();
	void Pause();
	void SendParameters();
	void SaveCalibration();
	void EnablePlay();
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
