#include "rendercircle.hpp"
#include "Filename.h"
#include <QFile>
#include "debug.h"
#include <qmath.h>

RenderCircle::RenderCircle(QWidget * parent) : QOpenGLWidget(parent) {
	_mesh.load(GetFullPath("models\\cube.ply").c_str());
	_rotating = false;
	_rotation.setToIdentity();
	_background = QColor(0, 0, 0);
	_gyroProgram = NULL;
}

RenderCircle::~RenderCircle() {
	
}

// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
static const GLfloat g_vertex_buffer_data[] = {
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f
};

// Two UV coordinatesfor each vertex. They were created withe Blender.
static const GLfloat g_uv_buffer_data[] = {
	0.000059f, 1.0f - 0.000004f,
	0.000103f, 1.0f - 0.336048f,
	0.335973f, 1.0f - 0.335903f,
	1.000023f, 1.0f - 0.000013f,
	0.667979f, 1.0f - 0.335851f,
	0.999958f, 1.0f - 0.336064f,
	0.667979f, 1.0f - 0.335851f,
	0.336024f, 1.0f - 0.671877f,
	0.667969f, 1.0f - 0.671889f,
	1.000023f, 1.0f - 0.000013f,
	0.668104f, 1.0f - 0.000013f,
	0.667979f, 1.0f - 0.335851f,
	0.000059f, 1.0f - 0.000004f,
	0.335973f, 1.0f - 0.335903f,
	0.336098f, 1.0f - 0.000071f,
	0.667979f, 1.0f - 0.335851f,
	0.335973f, 1.0f - 0.335903f,
	0.336024f, 1.0f - 0.671877f,
	1.000004f, 1.0f - 0.671847f,
	0.999958f, 1.0f - 0.336064f,
	0.667979f, 1.0f - 0.335851f,
	0.668104f, 1.0f - 0.000013f,
	0.335973f, 1.0f - 0.335903f,
	0.667979f, 1.0f - 0.335851f,
	0.335973f, 1.0f - 0.335903f,
	0.668104f, 1.0f - 0.000013f,
	0.336098f, 1.0f - 0.000071f,
	0.000103f, 1.0f - 0.336048f,
	0.000004f, 1.0f - 0.671870f,
	0.336024f, 1.0f - 0.671877f,
	0.000103f, 1.0f - 0.336048f,
	0.336024f, 1.0f - 0.671877f,
	0.335973f, 1.0f - 0.335903f,
	0.667969f, 1.0f - 0.671889f,
	1.000004f, 1.0f - 0.671847f,
	0.667979f, 1.0f - 0.335851f
};

void RenderCircle::LoadShaders()
{
	QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);

	bool compiled;
	{
		std::string str = GetFullPath("shaders\\gyro\\circleVertex.ogl");
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
		std::string str = GetFullPath("shaders\\gyro\\circleFragments.ogl");
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

	_gyroProgram = new QOpenGLShaderProgram;
	_gyroProgram->addShader(vshader);
	_gyroProgram->addShader(fshader);

	bool vbocreated = _vertexBuffer.create();
	DoAssert(vbocreated);
	bool vbobound = _vertexBuffer.bind();
	DoAssert(vbobound);
	_vertexBuffer.allocate(g_vertex_buffer_data, sizeof(g_vertex_buffer_data));

	_gyroProgram->link();

	_gyroProgram->bind();

	_vertexLocation = _gyroProgram->attributeLocation("vertexPosition_modelspace");
	_colorLocation = _gyroProgram->attributeLocation("color");

}

void RenderCircle::initializeGL() 
{
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);	
	LoadShaders();
}

void RenderCircle::paintGL() 
{
	if (!_gyroProgram)
	{
		glClearColor(0.5, 0.25, 0.4, _background.alphaF());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	emit reportSignal(MInfo, "Rendering frame");
	glClearColor(_background.redF(), _background.greenF(), _background.blueF(), _background.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// for each object that was split or loaded separately
	QMatrix4x4 m;
	m.perspective(3.14f / 2, 1, 0.5f, 10000.0f);
	m.translate(0.0f, 0.0f, -20);
	QVector3D vec(0, 0, 1);
	QVector3D vec2(cos(qDegreesToRadians(_azimuth)), sin(qDegreesToRadians(_azimuth)), 0);
	QVector3D axis = QVector3D::crossProduct(vec, vec2);
	m.rotate(_elevation, axis);

	///////////////////////////////////////
	/////////// setting uniform values
	///////////////////////////////////////

	_gyroProgram->setUniformValue("MVP", m);

	// not use right now, prepared for future use
	QMatrix4x4 id;
	id.setToIdentity();

	///////////////////////////////////////
	/////////// setting arrays
	///////////////////////////////////////

	// take vertex array from opengl context
	_gyroProgram->enableAttributeArray(_vertexLocation);
	_gyroProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	_gyroProgram->enableAttributeArray(_colorLocation);
	_gyroProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	///////////////////////////////////////
	/////////// Actual drawing
	///////////////////////////////////////
	int total = _mesh.getTrianglesList().size();
	for (int i = 0; i < total; ++i) {
		//textures[i]->bind();
		glDrawArrays(GL_TRIANGLE_FAN, i * 3, 3);
	}
}

void RenderCircle::mousePressEvent(QMouseEvent *event) 
{
	_rotating = true;
}

void RenderCircle::mouseMoveEvent(QMouseEvent *event) 
{
	if (!_rotating)
		return;
	
	emit rotationChanged(_rotation);
}

void RenderCircle::mouseReleaseEvent(QMouseEvent *event) 
{
	_rotating = false;
}
