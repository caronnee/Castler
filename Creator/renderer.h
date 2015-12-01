#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class Renderer : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  explicit Renderer(QWidget *parent = 0);
  ~Renderer();

  QSize minimumSizeHint() const Q_DECL_OVERRIDE;
  QSize sizeHint() const Q_DECL_OVERRIDE;
  void rotateBy(int xAngle, int yAngle, int zAngle);
  void setClearColor(const QColor &color);

signals:
  void clicked();

protected:
  void initializeGL() Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;
  void resizeGL(int width, int height) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
  void makeObject();

  QColor clearColor;
  QPoint lastPos;
  int xRot;
  int yRot;
  int zRot;
  //QOpenGLTexture *textures[6];
  QOpenGLShaderProgram *program;
  QOpenGLBuffer vbo;
};

#endif // RENDERER_H
