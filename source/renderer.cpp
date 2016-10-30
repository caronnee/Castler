/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderer.h"
#include "loghandler.h"
#include "debug.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include "Filename.h"

#define ZSTEP 0.03f
const float AngleStep = 0.05;

Renderer::Renderer(QWidget *parent)
  : QOpenGLWidget(parent),
  _background(Qt::white),
  program(0)
{
	_activeChange = 0;
	setFocusPolicy(Qt::ClickFocus);
	_movementFlag = 0;
	InitPosition();
}

void Renderer::ChangeShaders()
{
	CleanShaders();

	QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);

	bool compiled;
	{
		std::string str = GetFullPath("shaders\\vertexShader.ogl");
		QFile file(str.c_str());
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QByteArray content = file.readAll();

		const char * vertexShader = content.constData();
		compiled = vshader->compileSourceCode(vertexShader);
		file.close();
	}

	if (!compiled)
	{
		GLint maxLength = 0;
		QString ret = "VertexShader +" + vshader->log();
		bool shaderDidNotCompile = false;
		emit reportSignal(MError, ret);
		return;
	}

	{
		std::string str = GetFullPath("shaders\\fragmentShader.ogl");
		QFile file(str.c_str());
		file.open(QIODevice::ReadOnly | QIODevice::Text);

		QByteArray content = file.readAll();

		const char * fragmentShader = content.constData();
		compiled &= fshader->compileSourceCode(fragmentShader);
		const GLubyte * xstr = glGetString(GL_VERSION);
		file.close();
	}

	if (!compiled)
	{
		GLint maxLength = 0;
		QString ret = "Fragment shader:" + fshader->log();
		bool shaderDidNotCompile = false;
		emit reportSignal(MError, ret);
		return;
	}


	makeCurrent();

	program = new QOpenGLShaderProgram;
	program->addShader(vshader);
	program->addShader(fshader);

	program->bindAttributeLocation("vertexPosition_modelspace", 0);
	program->bindAttributeLocation("vertexNormal_modelspace", 1);

	// vertdata + normals
	QVector<GLfloat> vertData;
	CreateModels(vertData);

	bool vbocreated = _vertexBuffer.create();
	DoAssert(vbocreated);
	bool vbobound = _vertexBuffer.bind();
	DoAssert(vbobound);
	_vertexBuffer.allocate(vertData.data(), vertData.count()*sizeof(GLfloat));

	bool linked = program->link();

	if (!linked)
	{
		QString eeror = program->log();
		emit reportSignal(MError, eeror);
		delete program;
		program = NULL;
		return;
	}
	program->bind();
	update();
}

void Renderer::ChangeActiveKeyPos(int active)
{
	_activeChange = active;
}

void Renderer::InitPosition()
{
	_movementFlag = 0;
	memset(_desc, 0, sizeof(_desc));
	_desc[PositionCamera]._zPos = -2;
	_desc[PositionLight]._zPos = -15;
	// fixed so far
}

void Renderer::CleanShaders()
{
	if ( program )
	{
		makeCurrent();
		_vertexBuffer.destroy();
		delete program;
	}
	program = 0;
}

Renderer::~Renderer()
{
	CleanShaders();
	doneCurrent();
}

void Renderer::setClearColor(const QColor &color)
{
  _background = color;
  update();
}

void Renderer::initializeGL()
{
  initializeOpenGLFunctions();

  _name = GetFullPath("models\\cube.ply").c_str();

  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  ChangeShaders();

  _movementTimer.start(10, this);
}

static QMatrix4x4 CreateMatrix(PositionDesc & desc)
{
	QMatrix4x4 cameraMatrix;
	cameraMatrix.setToIdentity();
	QVector3D position(desc._xPos, desc._yPos, desc._zPos);
	cameraMatrix.rotate(desc._azimuth, 0, 0, 1);
	QVector3D vec(0, 0, 1);
	QVector3D vec2(cos(qDegreesToRadians(desc._azimuth)), sin(qDegreesToRadians(desc._azimuth)), 0);
	QVector3D axis = QVector3D::crossProduct(vec, vec2);
	cameraMatrix.rotate(desc._elevation, axis);
	cameraMatrix.translate(position);
	return cameraMatrix;
}

void Renderer::paintGL()
{
	if (!program)
	{
		glClearColor(0.5, 0.25, 0.4, _background.alphaF());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	emit reportSignal(MInfo, "Rendering frame");
	glClearColor(_background.redF(), _background.greenF(), _background.blueF(), _background.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// for each object that was split or loaded separately
	QMatrix4x4 modelMatrix = CreateMatrix(_desc[PositionModel]);
	QMatrix4x4 cameraMatrix = CreateMatrix(_desc[PositionCamera]);
	QMatrix4x4 perspectiveMatrix;
	float ratio = size().width() / (float)size().height();
	perspectiveMatrix.setToIdentity();
	perspectiveMatrix.perspective(90, ratio, .5f, 10000.0f);
	QMatrix4x4 mvp = perspectiveMatrix * cameraMatrix * modelMatrix;

	///////////////////////////////////////
	/////////// setting uniform values
	///////////////////////////////////////
	program->setUniformValue("modelToCamera", mvp);
	program->setUniformValue("viewMatrix", cameraMatrix);
	program->setUniformValue("modelMatrix", modelMatrix);
	QVector3D cameraPosition(_desc[PositionCamera]._xPos, _desc[PositionCamera]._yPos, _desc[PositionCamera]._zPos);
	program->setUniformValue("cameraPosition", cameraPosition);
	const float * df = _mesh.Diffuse();
	program->setUniformValue("MaterialDiffuseColor", QVector3D(df[0],df[1],df[2]) );
	// set 
	QVector3D lightPos(_desc[PositionLight]._xPos, _desc[PositionLight]._yPos, _desc[PositionLight]._zPos);
	program->setUniformValue("LightPosition_worldspace", lightPos);
	
	///////////////////////////////////////
	/////////// setting arrays
	///////////////////////////////////////

	// take vertex array from opengl context
	program->enableAttributeArray(0);
	program->enableAttributeArray(1);

	program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	program->setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	///////////////////////////////////////
	/////////// Actual drawing
	///////////////////////////////////////
	int total = _mesh.Triangles();
	for (int i = 0; i < total; ++i) {
		//textures[i]->bind();
		glDrawArrays(GL_TRIANGLE_FAN, i * 3, 3);
	}
}

void Renderer::keyPressEvent(QKeyEvent * e)
{
	SwitchKeys(e);
	e->accept();
}

void Renderer::SwitchKeys(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_W:
	{
		_movementFlag ^= MovementRotatePlusElevation;
		break;
	}
	case Qt::Key_S:
	{
		_movementFlag ^= MovementRotateMinusElevation;
		break;
	}
	case Qt::Key_A:
	{
		_movementFlag ^= MovementRotateMinusAzimuth;
		break;
	}
	case Qt::Key_D:
	{
		_movementFlag ^= MovementRotatePlusAzimuth;
		break;
	}
	case Qt::Key_Space:
	{
		InitPosition();
		
		update();
		// initial position
		break;
	}
	case Qt::Key_Down:
	{
		_movementFlag ^= MovementMinusDepth;
		break;
	}
	case Qt::Key_Up:
	{
		_movementFlag ^= MovementPlusDepth;
		break;
	}
	case Qt::Key_Left:
	{
		_movementFlag ^= MovementLeft;
		break;
	}
	case Qt::Key_Right:
	{
		_movementFlag ^= MovementRight;
		break;
	}
	case Qt::Key_Q:
	{
		_movementFlag ^= MovementUp;
		break;
	}
	case Qt::Key_Z:
	{
		_movementFlag ^= MovementDown;
		break;
	}
	}
}

void Renderer::keyReleaseEvent(QKeyEvent *e)
{
	SwitchKeys(e);
	e->accept();
}
void Renderer::timerEvent(QTimerEvent *e)
{
	if ((e->timerId() != _movementTimer.timerId()) || _movementFlag == 0 )
	{
		// no need to render
		return;
	}
	emit reportSignal(MInfo, "Checking setting");
	if (_movementFlag & MovementRotatePlusAzimuth)
		_desc[_activeChange]._azimuth += AngleStep;
	if (_movementFlag & MovementRotateMinusAzimuth)
		_desc[_activeChange]._azimuth -= AngleStep;
	if (_movementFlag & MovementRotatePlusElevation)
		_desc[_activeChange]._elevation += AngleStep;
	if (_movementFlag & MovementRotateMinusElevation)
		_desc[_activeChange]._elevation -= AngleStep;
	if (_movementFlag & MovementMinusDepth)
		_desc[_activeChange]._zPos -= ZSTEP;
	if (_movementFlag & MovementPlusDepth)
		_desc[_activeChange]._zPos += ZSTEP;
	if (_movementFlag & MovementLeft)
		_desc[_activeChange]._xPos -= ZSTEP;
	if (_movementFlag & MovementRight)
		_desc[_activeChange]._xPos += ZSTEP;
	if (_movementFlag & MovementUp)
		_desc[_activeChange]._yPos += ZSTEP;
	if (_movementFlag & MovementDown)
		_desc[_activeChange]._yPos -= ZSTEP;
	emit DescChangedSignal(_desc[_activeChange]);
	update();
}

void Renderer::mousePressEvent(QMouseEvent *event)
{
  lastPos = event->pos();
}

void Renderer::mouseMoveEvent(QMouseEvent *event)
{
  int dx = event->x() - lastPos.x();
  int dy = event->y() - lastPos.y();

  if (event->buttons() & Qt::LeftButton) {
	  _desc[_activeChange]._elevation += dy;
  } else if (event->buttons() & Qt::RightButton) {
	  _desc[_activeChange]._azimuth += dx;
  }
  lastPos = event->pos();
}

void Renderer::mouseReleaseEvent(QMouseEvent * /* event */)
{
  emit clicked();
}

void Renderer::CreateModels(QVector<GLfloat> & vertData)
{
	_mesh.Clear();
	_mesh.load(_name.toStdString());
	if (_mesh.getNumVertices() == 0)
	{
		DoAssert(false);
		return;
	}

	auto triangles = _mesh.Triangles();

	for (int i = 0; i < triangles;++i) {

		for (int j = 0; j < 3; j++)
		{
			cv::Point3f point = _mesh.getTriangleVertex(i, j);
			cv::Point3f normal = _mesh.GetNormal(i);
			vertData.append(point.x);
			vertData.append(point.y);
			vertData.append(point.z);
			vertData.push_back(normal.x);
			vertData.push_back(normal.y);
			vertData.push_back(normal.z);
			// no textures yet
		}
	}
}

void Renderer::Load(QString & str)
{
	_name = str;
	// TODO change to something that does not need to update shaders
	ChangeShaders();
}

void Renderer::ApplyDesc(const PositionDesc & desc)
{
	_desc[_activeChange] = desc;
}
