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
#include <QTimer>


#define ZSTEP 0.3f
const float AngleStep = 0.5;

Renderer::Renderer(QWidget *parent)
  : QOpenGLWidget(parent),
  clearColor(Qt::black),
  program(0)
{
	setFocusPolicy(Qt::ClickFocus);
//  memset(textures, 0, sizeof(textures));
	//QTimer *timer = new QTimer(this);
	//connect(timer, SIGNAL(timeout()), this, SLOT(Render()));
	//timer->start(5);
	InitPosition();
}
void Renderer::Render()
{
	update();
}

void Renderer::InitPosition()
{
	zPos = -20;
	elevation = 0;
	azimuth = 0;
}

Renderer::~Renderer()
{
  makeCurrent();
  vbo.destroy();
  //for (int i = 0; i < 6; ++i)
  //  delete textures[i];
  delete program;
  doneCurrent();
}

void Renderer::setClearColor(const QColor &color)
{
  clearColor = color;
  update();
}

#include "Filename.h"

void Renderer::initializeGL()
{
  initializeOpenGLFunctions();

  _name = GetFullPath("models\\bunny.ply").c_str();
  Init();

//  _mesh.ConvertToBB();
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

  QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
  const char *vsrc =
    "attribute highp vec4 vertex;\n"
    "uniform mediump mat4 matrix;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = matrix * vertex;\n"
    "}\n";
  vshader->compileSourceCode(vsrc);

  QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
  const char *fsrc =
	  "#version 100\n"
	  "void main()\n"
	  "{\n"
	  "    gl_FragColor = vec4(1,1,0,1);\n"
	  "}\n";
  bool compiled = fshader->compileSourceCode(fsrc);
  const GLubyte * str = glGetString(GL_VERSION);
  if (!compiled)
  {
	  GLint maxLength = 0;
	  QString ret = fshader->log();
	  bool shaderDidNotCompile = false;
	  DoAssert(shaderDidNotCompile);
	  //emit reportSignal(MError,ret);
	  return;
  }

  program = new QOpenGLShaderProgram;
  program->addShader(vshader);
  program->addShader(fshader);
  program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
  program->link();

  program->bind();
}

#include <qmath.h>

void Renderer::paintGL()
{
	emit reportSignal(MInfo, "Rendering frame");
	glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QMatrix4x4 m;
	m.perspective(3.14f/2, 1, 0.5f, 10000.0f);
	m.translate(0.0f, 0.0f, zPos);
	m.rotate(azimuth, 0, 0, 1);
	QVector3D vec(0, 0, 1);
	QVector3D vec2(cos(qDegreesToRadians(azimuth)), sin(qDegreesToRadians(azimuth)), 0);
	QVector3D axis = QVector3D::crossProduct(vec,vec2);
	m.rotate(elevation, axis);
	/*m.rotate(xRot , 1.0f, 0.0f, 0.0f);
	m.rotate(yRot , 0.0f, 1.0f, 0.0f);
	m.rotate(zRot , 0.0f, 0.0f, 1.0f);*/

	program->setUniformValue("matrix", m);
	program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
	program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
	int total = _mesh.getTrianglesList().size();
	for (int i = 0; i < total; ++i) {
		//textures[i]->bind();
		glDrawArrays(GL_TRIANGLE_FAN, i * 3, 3);
	}
}

void Renderer::keyPressEvent(QKeyEvent * e)
{
	switch (e->key())
	{
	case Qt::Key_W:
	{
		elevation += AngleStep;
		break;
	}
	case Qt::Key_S:
	{
		elevation -= AngleStep;
		break;
	}
	case Qt::Key_A:
	{
		azimuth -= AngleStep;
		break;
	}
	case Qt::Key_D:
	{
		azimuth += AngleStep;
		break;
	}
	case Qt::Key_Space:
	{
		InitPosition();
		// initial position
		break;
	}
	case Qt::Key_Down:
	{
		zPos-=ZSTEP;
		break;
	}
	case Qt::Key_Up:
	{
		zPos += ZSTEP;
		break;
	}
	default:
		return; // no need updating
	}
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
    elevation += dy;
  } else if (event->buttons() & Qt::RightButton) {
    azimuth += dx;
  }
  lastPos = event->pos();
}

void Renderer::mouseReleaseEvent(QMouseEvent * /* event */)
{
  emit clicked();
}

void Renderer::makeObject()
{
	if (_mesh.getNumVertices() == 0)
	{
		DoAssert(false);
		return;
	}
  //for (int j = 0; j < 6; ++j)
  //  textures[j] = new QOpenGLTexture(QImage(QString(":/images/side%1.png").arg(j + 1)).mirrored());

  QVector<GLfloat> vertData;

  auto triangles = _mesh.getTrianglesList();
  cv::Point3f mn = _mesh.getVertex(0), mx = _mesh.getVertex(0);
  for (int i = 0; i < triangles.size(); ++i) {
      // vertex position
	  DoAssert(triangles[i].size() == 3);

	  for ( int j = 0; j < triangles[i].size(); j++)
	  {
		  cv::Point3f point = _mesh.getVertex( triangles[i][j] );
		  vertData.append(point.x);
		  vertData.append(point.y);
		  vertData.append(point.z);
		  // no textures yet
	  }
  }

  vbo.create();
  vbo.bind();
  vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
}

void Renderer::Clear()
{
	vbo.destroy();
}

void Renderer::Init()
{
	Clear();
	_mesh.load(_name.toStdString());
	makeObject();
}

void Renderer::Load(QString & str)
{
	_name = str;
	Init();
}
