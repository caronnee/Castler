#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include "model/Mesh.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

#include "ReportFunctions.h"

class Renderer : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  explicit Renderer(QWidget *parent = 0);
  ~Renderer();

  void rotateBy(int xAngle, int yAngle, int zAngle);
  void setClearColor(const QColor &color);

signals:
  void clicked();
  void reportSignal(MessageLevel, const QString& string);

private slots:
  void rotateOneStep();

protected:
  void initializeGL() Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;
  void keyPressEvent(QKeyEvent *);
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
  void makeObject();

  Mesh _mesh;
  QString _name;
  QColor clearColor;
  QPoint lastPos;
  int xRot;
  int yRot;
  int zRot;
  //QOpenGLTexture *textures[6];
  QOpenGLShaderProgram *program;
  QOpenGLBuffer vbo;

private:
	void Clear();;
	void Init();
public:
	void Load(QString & str);
};

#endif // RENDERER_H
