#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QBasicTimer>
#include "model/Mesh.h"
#include <QVector3D>
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

class Renderer : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
		;

	int _activeChange;

	PositionDesc _desc[NKeyPositions];
public:

	explicit Renderer(QWidget *parent = 0);
	void InitPosition();

	void CleanShaders();
	~Renderer();

	void setClearColor(const QColor &color);

signals:
	void clicked();
	void reportSignal(MessageLevel, const QString& string);

	private slots:
	void ChangeShaders();
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
	void CreateModels(QVector<GLfloat>& vertData);

	// timer to change/animate according to movement flag
	QBasicTimer _movementTimer;

	// mesh to show - TODo later to replace
	Mesh _mesh;

	// name of the model shown
	QString _name;

	// color outside detected model
	QColor _background;

	// last mouse position
	QPoint lastPos;

	//QOpenGLTexture *textures[6];
	QOpenGLShaderProgram *program;

	// buffers
	QOpenGLBuffer _vertexBuffer;

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
};

#endif // RENDERER_H
