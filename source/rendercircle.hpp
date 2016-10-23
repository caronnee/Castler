#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include "ReportFunctions.h"
#include "model/Mesh.h"

class RenderCircle : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

		;
private:

	QOpenGLBuffer _vertexBuffer,_uvBuffer;

	int _vertexLocation, _colorLocation;

	// color outside detected model
	QColor _background;
	float _elevation, _azimuth;
	QOpenGLShaderProgram * _gyroProgram;

	bool _rotating;

	QMatrix4x4 _rotation;

	// cube where to 
	Mesh _mesh;

private:
	void LoadShaders();

signals:
	void rotationChanged(QMatrix4x4);
	void reportSignal(MessageLevel, const QString& string);

public:
	RenderCircle(QWidget * parent = Q_NULLPTR);
	~RenderCircle();
	
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

};
