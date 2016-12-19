#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QBasicTimer>
#include "model/Mesh.h"
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <qmath.h>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

#include "ReportFunctions.h"

enum PositionOnKeys
{
	PositionCamera,
	PositionLight,
	PositionModel,
	NKeyPositions
};

struct RenderData
{
	//name of the filename / mesh
	QString _name;

	// mesh to show - TODo later to replace
	Mesh _mesh;

	// constructor. Just initialize the name for later loading
	RenderData(const QString & name);

	// physical load data to mesh
	void Load();
};

// how the mesh should be displayed
enum RenderStyle
{
	RenderPoints,
	RenderWireframe,
	RenderComplete,
	NRenders
};

class Renderer : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
		;

	// index of the change
	int _activeChange;

	// description of the extrinsic parameters of the meshes
	PositionDesc _desc[NKeyPositions];

	int _indices;

	// style that should be used for rendering
	int _renderStyle;

	// buffers
	QOpenGLBuffer _vertexBuffer;

	QOpenGLBuffer _indicesBuffer;

	bool ReadShader(QOpenGLShader * shader, const char * name);

public:

	explicit Renderer(QWidget *parent = 0);
	void InitPosition();

	void CleanShaders();
	~Renderer();

	void setClearColor(const QColor &color);

signals:
	void clicked();
	void reportSignal(MessageLevel, const QString& string);
	void DescChangedSignal(PositionDesc&);

private slots:
	void ChangeShaders();

public slots:
	void ChangeRenderStyle(int);
	void ChangeActiveKeyPos(int);

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *);

	void SwitchKeys(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);

	void timerEvent(QTimerEvent *e);
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
	void CreateModels();

	// timer to change/animate according to movement flag
	QBasicTimer _movementTimer;

	std::vector<RenderData> _renderData;

	// color outside detected model
	QColor _background;

	// last mouse position
	QPoint lastPos;

	//QOpenGLTexture *textures[6];
	QOpenGLShaderProgram *_shaderProgram;

	// modes for animation of selected object
	enum RenderMovements
	{
		MovementRotatePlusAzimuth = 1,
		MovementRotateMinusAzimuth = 2,
		MovementRotatePlusElevation = 4,
		MovementRotateMinusElevation = 8,
		MovementPlusDepth = 16,
		MovementMinusDepth = 32,
		MovementLeft = 64,
		MovementRight = 128,
		MovementUp = 256,
		MovementDown = 512,
		NMovements
	};

	// animation flags
	int _movementFlag;

public:
	void Load(QString & str);
	void ApplyDesc(const PositionDesc & desc);
};

#endif // RENDERER_H
