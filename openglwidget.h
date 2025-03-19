#ifndef __OpenGLWidget_H__
#define __OpenGLWidget_H__



#include <QColor>
#include <QFont>
#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPainter>
#include <QVector3D>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include <QMenuBar>
#include <QMainWindow>
#include <QMessageBox>
#include <vector>

#include <QApplication>
#include <QObject>
#include <QString>
//#include <GL/glu.h>
#include <QApplication>
#include <QMouseEvent>
#include <QFile>
#include "renderpoint.h"
#include <vector>


#include <QDebug>
#include <QList>
#include <QImage>

/*#ifdef DEBUG
#include <iostream>opeoo
#include <QDebug>
*/

class RPoint : public QObject
{
    Q_OBJECT

public:
    QVector3D center = QVector3D(0, 0, 0);
    float radius = 1;
    QColor color;
    float transparent=0.0f;


};

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    //Q_PROPERTY(bool Is3D READ is3D WRITE setIs3D NOTIFY optionsChanged)
    Q_PROPERTY(float MouseWheelSensitivity READ mouseWheelSensitivity WRITE setMouseWheelSensitivity)
    Q_PROPERTY(bool SwapMouseWheelZoomDirection READ swapMouseWheelZoomDirection WRITE setSwapMouseWheelZoomDirection)
  //  Q_PROPERTY(QColor BackgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY optionsChanged)
  //  Q_PROPERTY(QFont HudFont READ hudFont WRITE setHudFont NOTIFY optionsChanged)
    
public:
   void reloadSomePoints(int begin,int end);
       void reloadPoints(int loaddir);


    void createMenu(QMainWindow &MainWindow);
    float _cameraSpeed=1.0;
    OpenGLWidget(QWidget *parent = NULL) : QOpenGLWidget(parent) {}
    OpenGLWidget(std::vector<RenderPoint> &pointcloud2,int camID );



    virtual ~OpenGLWidget() {}
    
    struct Camera {
        QVector3D eye = QVector3D(0, 0, 10);
        QVector3D center = QVector3D(0, 0, 0);
        QVector3D up = QVector3D(0, 1, 0);
        QVector3D right = QVector3D(0, 0, 1);
        QVector3D front = QVector3D(1, 0, 0);
        QVector3D view() { return center - eye; }
        void zoom(float viewDistance) { eye = center - view().normalized() * viewDistance; }
    } camera;
    
    bool is3D() const { return _is3D; }
    void setIs3D(bool b) { if(_is3D && !b) goToDefaultView(); _is3D = b; }
    
    float mouseWheelSensitivity() const { return _mouseWheelSensitivity; }
    void setMouseWheelSensitivity(float f) { _mouseWheelSensitivity = f > 1e-3 ? f : 1e-3; }
    
    bool swapMouseWheelZoomDirection() const { return _swapMouseWheelZoomDirection; }
    void setSwapMouseWheelZoomDirection(bool b) { _swapMouseWheelZoomDirection = b; }
    
    QColor backgroundColor() const { return _backgroundColor; }
    void setBackgroundColor(const QColor &color) { _backgroundColor = color; }
    
    QFont hudFont() const { return _hudFont; }
    void setHudFont(const QFont &font) { _hudFont = font; }
    
    // useful stuff
    static QVector3D screen2World(QVector3D screen, int *viewport, float *projection, float *modelview);
    static QVector3D world2Screen(QVector3D world, int *viewport, float *projection, float *modelview);
    static float intersectRayAndRPoint(const QVector3D &rayOrigin, const QVector3D &rayDirection, const QVector3D &sphereCenter, float sphereRadius);
    static float intersectRayAndPlane(const QVector3D &rayOrigin, const QVector3D &rayDirection, const QVector3D &pointOnPlane, const QVector3D &planeNormal);
    void goToBillboard(const QVector3D &origin, const QVector3D &right);
    bool isLeftToRight(const QVector3D &vec);
    static float luminance(const QColor &color);
    static QColor colorWithMaxContrast(const QColor &color);
    void renderText(float x, float y, const QString &text, const QColor &color, const QFont &font);
    
    // drawing
    virtual void drawScene();

    virtual void drawHud(QPainter &painter);
    void drawAxes();
    
    // mouse selection
    virtual void selectObject(const QPoint &mousePosition);
    void getPickRay(const QPoint &mousePosition, QVector3D &origin, QVector3D &ray);
    QVector3D pickPointInPlane(const QPoint &mousePosition, const QVector3D &pointOnPlane, bool snapToUnitGrid = false);

    virtual void goToDefaultView();
    virtual void deleteSelectedObject();
    virtual void editSelectedObject(const QPoint &mousePosition);


    //try call other instances
    std::vector<OpenGLWidget *> renderwidgets;
    void add_subwidget(OpenGLWidget opglw);




    //parts of road
    int part_stage4=0;
    int piece_size=25;
    int start_point=1000;
    int camID_gl;

    float offset_x=-493477.0,offset_y=-4982660.0,offset_z=-19.0;

    std::vector<RenderPoint> pointcloud;
    float rpointsize = 0.3;

    float xrot=0.0, yrot=0.0,zrot=0.0;

    int loadData();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    
    bool _is3D = true;
    float _mouseWheelSensitivity = 0.2;
    bool _swapMouseWheelZoomDirection = false;
    QColor _backgroundColor = QColor(20, 20, 20);
    QFont _hudFont = QFont("Sans", 10, QFont::Normal);
    QPoint _mousePosition;
    QObject *_selectedObject = NULL;

};

#endif
