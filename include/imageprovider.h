#pragma once

#include <QString>
#include <QMetaType>
#include <opencv2/opencv.hpp>

enum VisualModes
{
	VisualModeStop = 0,
	VisualModePlay = 1 << 1,
	VisualModeGrey = 1 << 2,
	VisualModeFeatures = 1 << 3,
	VisualModeUndistort = 1 << 4,
};

// no need to make mask of this - according to these flags processor is created
enum ActionModes
{
	ActionModePlay, // do nothing
	ActionModeCalibrate,
	ActionModeCreate,
	ActionModeCreateManual,
	NActions
};
#include "typedefs.h"
struct PointsContext
{
	// boll if the keypoints are provided and so
	bool mode;

	// ids of the images
	int id[2];

	// 2D coordinated in the pictures
	CoordsArray coords;

	// indexes to coords
	std::vector<int> indexes;

	// additional data
	int data;

	// used description - TODO remove. We have ids
	QString description;
public:
	void Set(const PointsContext & ctx)
	{
		id[0] = ctx.id[0];
		id[1] = ctx.id[1];
		coords = ctx.coords;
		data = ctx.data;
		description = ctx.description;
		indexes = ctx.indexes;
		mode = ctx.mode;
	}

	PointsContext() : mode(false) {
	}
	PointsContext(const PointsContext & ctx) : mode(false) {
		Set(ctx);
	}
	PointsContext & operator=(PointsContext & ctx)
	{
		Set(ctx);
		return *this;
	}
	void Clear();	
};

struct CalibrationSet
{
	float px, py, k1, k2, k3, fx, fy;
	CalibrationSet()
	{
		px = 0, py = 0, k1 = 0, k2 = 0, k3 = 0, fx = 20, fy = 20;
	}
};

Q_DECLARE_METATYPE(CalibrationSet)

class IImageProvider
{
public:
	virtual bool IsValid() = 0;
	virtual bool Frame(cv::Mat & frame)=0;
	virtual void PreviousFrame(cv::Mat & frame)=0;
	virtual void SetPosition(const int & position) = 0;
	virtual bool Next() = 0;
	virtual int Position() = 0;
	virtual double Step() = 0;
	virtual int Count() = 0;
	virtual const QString Name()const = 0;
	virtual const QString Name(const int & pos) const=0;
	virtual void SkipCurrent() = 0;
};

struct Providers
{
	std::vector<IImageProvider *> _providers;
	int _currentProvider = 0;

	// sets provider to 0
	void Reset();

	//number of frames this provides
	int Count();

	// set position
	void SetPosition(const int & position);

	// fins position to be set
	void FindPosition(int & position, int & provider);

	// sets positino and immediately retrieves frame
	void Get(int position, cv::Mat& frame);

	// retrieves frame
	bool Frame(cv::Mat& frame);

	// move one forward
	bool Next();
	void SkipCurrent()
	{
		_providers[_currentProvider]->SkipCurrent();
	}
	int Step()
	{
		return _providers[_currentProvider]->Step();
	}
	
	// used for destructor
	void Clear();

	// created providers
	void Create(const QString & str);

	bool IsValid() const;

	// current nae of the processed frame
	const QString Name();
	const QString Name(const int & id);
};


IImageProvider * CreateProvider(const QString & source); 
bool IsSupportedFile(const QString & source);