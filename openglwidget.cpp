#include "openglwidget.h"

#include <climits>
#include <cmath>

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMessageBox>
#include <QMouseEvent>
#include <QVector4D>
#include <QWheelEvent>



void OpenGLWidget::reloadSomePoints(int begin,int end){

    int rp = 100;

    //qDebug()<<pointcloud[0].x<<" "<<" "<<pointcloud[0].y<<" "<<pointcloud[0].z;
    //position offset
    offset_x=-1*pointcloud[0].x;
    offset_y=-1*pointcloud[0].y;
        // offset_z=-1*pointcloud[0].z;
}

void OpenGLWidget::reloadPoints(int loaddir){

    int rp = 100;

    //qDebug()<<pointcloud[0].x<<" "<<" "<<pointcloud[0].y<<" "<<pointcloud[0].z;
    //position offset
    offset_x=-1*pointcloud[0].x;
    offset_y=-1*pointcloud[0].y;
    offset_z=-1*pointcloud[0].z;



}

QVector3D OpenGLWidget::screen2World(QVector3D screen, int *viewport, float *projection, float *modelview)
{
    QMatrix4x4 P(projection);
    QMatrix4x4 M(modelview);
    P = P.transposed(); // row to column major order
    M = M.transposed(); // row to column major order
    double x = screen.x();
    double y = viewport[3] - screen.y();
    double z = screen.z();
    QVector4D in(
        2 * (x - viewport[0]) / viewport[2] - 1, // Map between (-1,1)
        2 * (y - viewport[1]) / viewport[3] - 1, // Map between (-1,1)
        2 *  z                              - 1, // Fed in as 0 or 1 which maps to (-1,1)
        1
    );
    QVector4D out = (P * M).inverted() * in;
    if(out[3] == 0)
        throw std::runtime_error("OpenGLWidget::screen2World: Failed.");
    x = out[0] / out[3];
    y = out[1] / out[3];
    z = out[2] / out[3];
    return QVector3D(x, y, z);
}

QVector3D OpenGLWidget::world2Screen(QVector3D world, int *viewport, float *projection, float *modelview)
{
    QMatrix4x4 P(projection);
    QMatrix4x4 M(modelview);
    P = P.transposed(); // row to column major order
    M = M.transposed(); // row to column major order
    QVector4D A(world.x(), world.y(), world.z(), 1);
    QVector4D B = (M * P) * A;
    if(B[3] == 0)
        throw std::runtime_error("OpenGLWidget::world2Screen: Failed.");
    B[0] /= B[3];
    B[1] /= B[3];
    B[2] /= B[3];
    double x = viewport[0] + ((B[0] + 1) * viewport[2]) / 2; // Map from (-1,1)
    double y = viewport[1] + ((B[1] + 1) * viewport[3]) / 2; // Map from (-1,1)
    double z = (1 + B[2]) / 2; // Map from (-1,1) to (0,1)
    return QVector3D(round(x), round(viewport[3] - y), z);
}

float OpenGLWidget::intersectRayAndRPoint(const QVector3D &rayOrigin, const QVector3D &rayDirection, const QVector3D &sphereCenter, float sphereRadius)
{
    QVector3D L = sphereCenter - rayOrigin;
    float tca = QVector3D::dotProduct(L, rayDirection);
    if(tca < 0) return -1;
    float d2 = QVector3D::dotProduct(L, L) - (tca * tca);
    float r2 = sphereRadius * sphereRadius;
    if(d2 > r2) return -1;
    float thc = sqrt(r2 - d2);
    return tca - thc;
}

float OpenGLWidget::intersectRayAndPlane(const QVector3D &rayOrigin, const QVector3D &rayDirection, const QVector3D &pointOnPlane, const QVector3D &planeNormal)
{
    float numerator = QVector3D::dotProduct(planeNormal, pointOnPlane - rayOrigin);
    float denominator = QVector3D::dotProduct(planeNormal, rayDirection);
    return (fabs(denominator) > 1e-5 ? numerator / denominator : -1);
}

void OpenGLWidget::goToBillboard(const QVector3D &origin, const QVector3D &right)
{
    QVector3D xhat = (right - origin).normalized();
    QVector3D yhat = QVector3D::crossProduct(xhat, camera.view()).normalized();
    QVector3D zhat = QVector3D::crossProduct(xhat, yhat).normalized();
    // Rotation to (x, y, z) and translation to origin.
    float transform[16] = {
        xhat.x(), xhat.y(), xhat.z(), 0,
        yhat.x(), yhat.y(), yhat.z(), 0,
        zhat.x(), zhat.y(), zhat.z(), 0,
        origin.x(), origin.y(), origin.z(), 1
    };
    glMultMatrixf(transform);
}

bool OpenGLWidget::isLeftToRight(const QVector3D &vec)
{
    QVector3D right = QVector3D::crossProduct(camera.view(), camera.up);
    float dpr = QVector3D::dotProduct(right.normalized(), vec);
    float len = vec.length();
    float epsilon = 1e-5 * len;
    if(fabs(dpr) < epsilon) {
        // straight up-down
        float dpu = QVector3D::dotProduct(camera.up.normalized(), vec);
        if(-dpu > len - epsilon) {
            return false; // straight down
        } else {
            return true; // straight up
        }
    }
    return dpr > 0;
}



float OpenGLWidget::luminance(const QColor &color)
{
    float r = color.redF();
    float g = color.greenF();
    float b = color.blueF();
    if(r <= 0.03928) r /= 12.92; else r = pow((r + 0.055) / 1.055, 2.4);
    if(g <= 0.03928) g /= 12.92; else g = pow((g + 0.055) / 1.055, 2.4);
    if(b <= 0.03928) b /= 12.92; else b = pow((b + 0.055) / 1.055, 2.4);
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

QColor OpenGLWidget::colorWithMaxContrast(const QColor &color)
{
    float hue = color.hueF() + 0.5;
    if(hue > 1) hue -= 1;
    return QColor::fromHslF(hue, 1, 0.5);
}

void OpenGLWidget::renderText(float x, float y, const QString &text, const QColor &color, const QFont &font)
{
    // QPainter changes one or more attributes so all relevent ones are pushed then popped on end of QPainter.
    glPushAttrib(GL_ACCUM_BUFFER_BIT);
    glPushAttrib(GL_VIEWPORT_BIT);
    glPushAttrib(GL_TRANSFORM_BIT);
    glPushAttrib(GL_POLYGON_BIT);
    glPushAttrib(GL_PIXEL_MODE_BIT);
    glPushAttrib(GL_MULTISAMPLE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glPushAttrib(GL_CURRENT_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    QPainter painter(this);
    painter.setPen(color);
    painter.setFont(font);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(x, y, text);
    painter.end();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

void OpenGLWidget::drawScene()
{
    //drawAxes();

    for(auto &pcloud: pointcloud){

        glPushMatrix();
         if(camID_gl==3)
            glRotatef(90,-1.48,-0.29,1);

        /*
        if(camID_gl==2){
            glTranslatef( pcloud.y+offset_y,pcloud.x + offset_x,pcloud.z+offset_z);

        }else if(camID_gl==3){

        }else{
            glTranslatef(pcloud.x + offset_x, pcloud.y+offset_y,pcloud.z+offset_z);
}*/

      //  if(camID_gl==3){
       //      glTranslatef( pcloud.y + offset_y,pcloud.z+offset_z,pcloud.x+offset_x);
      //  }else




        glTranslatef(pcloud.x + offset_x, pcloud.y+offset_y,pcloud.z+offset_z);
        glColor3d(pcloud.r/100.0,pcloud.g/100.0,pcloud.b/100.0);
        //  glTranslatef(pcloud.x , pcloud.y,pcloud.z);
        glBegin(GL_TRIANGLES);
        glVertex3f(- rpointsize ,0.0,rpointsize/2);
        glVertex3f( rpointsize ,0.0,0.0);
        glVertex3f(0.0,rpointsize,0.0);
        glEnd();
        /*
            glBegin(GL_TRIANGLES);
            glVertex3f(0.0f,0.0,-rpointsize);
            glVertex3f(rpointsize/2,0.0,rpointsize);
            glVertex3f(0.0,rpointsize,0.0);
            glEnd();
*/
        glPopMatrix();


        /*
        if(camID_gl==3){
            QImage img_temp = grabFramebuffer();
            QTransform tr;
            tr.rotate(90);
            img_temp=  img_temp.transformed(tr);
            glDrawPixels(img_temp.height(),img_temp.width(), GL_RGBA,GL_UNSIGNED_BYTE, img_temp.bits());
        }
*/
    }

}


void OpenGLWidget::drawHud(QPainter &painter)
{
    QString text;
  //  if(is3D()) text = "Mouse: M=pan, R=rot, W=zoom";
//  else text = "Mouse: M,R=pan, W=zoom";
    if(!text.isEmpty()) {
        QFontMetricsF fm(_hudFont);
        QRectF bbox = fm.tightBoundingRect(text);
        float x = bbox.bottomLeft().x();
        float y = bbox.bottomLeft().y();
        float h = bbox.height();
        QColor textColor = luminance(_backgroundColor) > 0.25 ? QColor(0, 0, 0) : QColor(255, 255, 255); // WC3 guidlines is L > ~0.179
        painter.setPen(textColor);
        painter.setFont(_hudFont);
        painter.drawText(2 - x, 2 - y + h, text);
    }
}

void OpenGLWidget::drawAxes()
{
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glPushAttrib(GL_LINE_BIT);
    glLineWidth(4);
    glBegin(GL_LINES);
    // x
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    // y
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    // z
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();
    glPopAttrib(); // GL_LINE_BIT
    glPopAttrib(); // GL_COLOR_BUFFER_BIT
}

void OpenGLWidget::selectObject(const QPoint &mousePosition)
{
//    QVector3D pickOrigin, pickRay;
//    getPickRay(mousePosition, pickOrigin, pickRay);
//    _selectedObject = NULL;
 
}

void OpenGLWidget::getPickRay(const QPoint &mousePosition, QVector3D &origin, QVector3D &ray)
{
    makeCurrent();
    int viewport[4] = {0, 0, width(), height()};
    float projection[16];
    float modelview[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    origin = screen2World(QVector3D(mousePosition.x(), mousePosition.y(), 0), viewport, projection, modelview);
    ray = screen2World(QVector3D(mousePosition.x(), mousePosition.y(), 1), viewport, projection, modelview) - origin;
}

QVector3D OpenGLWidget::pickPointInPlane(const QPoint &mousePosition, const QVector3D &pointOnPlane, bool snapToUnitGrid)
{
    QVector3D pickOrigin, pickRay;
    getPickRay(mousePosition, pickOrigin, pickRay);
    pickRay.normalize();
    float t = intersectRayAndPlane(pickOrigin, pickRay, pointOnPlane, camera.view().normalized());
    if(t >= 0) {
        QVector3D pt = pickOrigin + (pickRay * t);
        if(snapToUnitGrid) {
            pt.setX(round(pt.x()));
            pt.setY(round(pt.y()));
            pt.setZ(round(pt.z()));
        }
    }
    return pointOnPlane;
}

void OpenGLWidget::goToDefaultView()
{
    camera.eye = QVector3D(0, 0, 1);
    camera.center = QVector3D(0, 0, 0);
    camera.up = QVector3D(0, 1, 0);
    repaint();
}

void OpenGLWidget::deleteSelectedObject()
{
    if(!_selectedObject) return;
    QString title("Delete selected object?");
    QString text("Delete " + _selectedObject->objectName() + "?");
    if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    delete _selectedObject;
    _selectedObject = NULL;
   // emit selectedObjectChanged(_selectedObject);
    repaint();
}

void OpenGLWidget::editSelectedObject(const QPoint &mousePosition)
{
    return;
    // this doesn't really do anything useful, but provides a simple example of how you might popup an editor widget for your scene objects
    if(!_selectedObject) return;
    QWidget *editor = new QWidget;
    editor->setWindowModality(Qt::ApplicationModal);
    editor->setWindowTitle(_selectedObject->metaObject()->className());
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->show();
}

QOpenGLShaderProgram *shaderProgram;
GLuint vao;
GLuint vbo;

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(_backgroundColor.redF(), _backgroundColor.greenF(), _backgroundColor.blueF(), _backgroundColor.alphaF());
     glEnable(GL_DEPTH_TEST);
    // Set up the shaders

}

void OpenGLWidget::resizeGL(int /* w */, int /* h */)
{
    glViewport(0, 0, width(), height());

}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // ortho projection (call here to handle zoom)
    // ortho box is sized relative to zoom distance
    float zoom = camera.view().length();
    float left = -zoom / 2;
    float right = zoom / 2;
    float bottom = -zoom / 2;
    float top = zoom / 2;
    float near = zoom / 100;
    float far = zoom * 2;
    float aspect = float(width()) / height();
    if(aspect < 1) {
        bottom /= aspect;
        top /= aspect;
    } else if(aspect > 1) {
        left *= aspect;
        right *= aspect;
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left, right, bottom, top, near, far);

    // lookat
    QVector3D zhat = -camera.view().normalized();
    QVector3D xhat = QVector3D::crossProduct(camera.up, zhat).normalized();
    QVector3D yhat = QVector3D::crossProduct(zhat, xhat).normalized();
    float eyeX = QVector3D::dotProduct(camera.eye, xhat);
    float eyeY = QVector3D::dotProduct(camera.eye, yhat);
    float eyeZ = QVector3D::dotProduct(camera.eye, zhat);
    float lookat[16] = {
        xhat.x(), yhat.x(), zhat.x(), 0,
        xhat.y(), yhat.y(), zhat.y(), 0,
        xhat.z(), yhat.z(), zhat.z(), 0,
        -eyeX,    -eyeY,    -eyeZ,    1
    };
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(lookat);

    drawScene();
    
    // text info

    glPushAttrib(GL_ACCUM_BUFFER_BIT);
    glPushAttrib(GL_VIEWPORT_BIT);
    glPushAttrib(GL_TRANSFORM_BIT);
    glPushAttrib(GL_POLYGON_BIT);
    glPushAttrib(GL_PIXEL_MODE_BIT);
    glPushAttrib(GL_MULTISAMPLE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glPushAttrib(GL_CURRENT_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawHud(painter);
    painter.end();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();


}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    QVector3D translation(0.0,0.0,0.0);
    switch(event->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
            deleteSelectedObject();
            return;

        case Qt::Key_Escape:
            close();
            return;
            
        case Qt::Key_C:
            goToDefaultView();
            return;
            /*
        case Qt::Key_W:
            camera.eye +=QVector3D(1.0,0.0,0.0);
            repaint();
            return;

        case Qt::Key_S:
             camera.eye +=QVector3D(-1.0,0.0,0.0);
            repaint();
            return;

        case Qt::Key_A:
            camera.eye +=QVector3D(0.0,1.0,0.0);
            repaint();
            return;

        case Qt::Key_D:
            camera.eye +=QVector3D(0.0,-1.0,0.0);
            repaint();
            return;

        case Qt::Key_Z:
            camera.eye +=QVector3D(0.0,0.0,1.0);
            repaint();
            return;

        case Qt::Key_Space:
            camera.eye +=QVector3D(0.0,0.0,-1.0);
            repaint();
            return;
*/
            

        case Qt::Key_W:
          translation = QVector3D(0.0,0.0,_cameraSpeed);
          camera.center -= translation;
          camera.eye -= translation;
          repaint();
          return;
           
        case Qt::Key_S:
          translation =  QVector3D(0.0,0.0,-_cameraSpeed);
          camera.center -= translation;
          camera.eye -= translation;
          repaint();
          return;
           
        case Qt::Key_A:
          translation = QVector3D(_cameraSpeed,0.0,0.0);
          camera.center -= translation;
          camera.eye -= translation;
          repaint();
          return;
           
        case Qt::Key_D:
          translation = QVector3D(-_cameraSpeed,0.0,0.0);
          camera.center -= translation;
          camera.eye -= translation;
          repaint();
          return;
           
        case Qt::Key_Z:
          translation = QVector3D(0.0,_cameraSpeed,0.0);
          camera.center -= translation;
          camera.eye -= translation;
          repaint();
          return;
           
        case Qt::Key_Space:
          translation = QVector3D(0.0,-_cameraSpeed,0.0);
          camera.center -= translation;
          camera.eye -= translation;
          repaint();
          return;

        case Qt::Key_PageUp:
          reloadPoints(2);
          repaint();
            part_stage4++;


          renderwidgets[0]->reloadPoints(2);
            renderwidgets[0]->repaint();
          renderwidgets[0]->part_stage4++;

            renderwidgets[1]->reloadPoints(2);
            renderwidgets[1]->repaint();
            renderwidgets[1]->part_stage4++;

            renderwidgets[2]->reloadPoints(2);
            renderwidgets[2]->repaint();
            renderwidgets[2]->part_stage4++;
        // drawScene();


          return;

        case Qt::Key_PageDown:
         reloadPoints(0);
          repaint();
            part_stage4--;


          renderwidgets[0]->reloadPoints(0);
          renderwidgets[0]->repaint();
          renderwidgets[0]->part_stage4--;

          renderwidgets[1]->reloadPoints(0);
          renderwidgets[1]->repaint();
          renderwidgets[1]->part_stage4--;


          renderwidgets[2]->reloadPoints(0);
          renderwidgets[2]->repaint();
          renderwidgets[2]->part_stage4--;


          return;
    }
    QOpenGLWidget::keyPressEvent(event);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        // object selection
        QObject *prevSelectedObject = _selectedObject;
        selectObject(event->pos());
        if(_selectedObject != prevSelectedObject) {
          //  emit selectedObjectChanged(_selectedObject);
        }
        if(_selectedObject) {
            // for dragging object
            _mousePosition = event->pos();
            setMouseTracking(true);
        }
        repaint();
        return;
    } else if(event->button() == Qt::MiddleButton || (!is3D() && (event->button() == Qt::RightButton))) {
        // pan
        _mousePosition = event->pos();
        setMouseTracking(true);
        return;
    } else if(event->button() == Qt::RightButton) {
        if(is3D()) {
            // rotate
            _mousePosition = event->pos();
            setMouseTracking(true);
            return;
        }
    }
    QOpenGLWidget::mousePressEvent(event);
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(true) {
        setMouseTracking(false);
        return;
    }
    QOpenGLWidget::mousePressEvent(event);
}


void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Transform scene.
    bool rotate = is3D() && (event->buttons() & Qt::RightButton);
    bool pan = event->buttons() & Qt::MiddleButton || (!is3D() && (event->buttons() & Qt::RightButton));
    if(rotate || pan) {
        float dx = -( event->x() - _mousePosition.x());
        float dy = (event->y() - _mousePosition.y());
        _mousePosition = event->pos();
        QVector3D zhat = -camera.view().normalized();
        QVector3D xhat = QVector3D::crossProduct(camera.up, zhat).normalized();
        QVector3D yhat = QVector3D::crossProduct(zhat, xhat).normalized();
        if(rotate) {
          //  qDebug()<<" "<<yhat * dx<<" "<<xhat * dy<<" ";
            QVector3D rotationAxis = yhat * dx - xhat * dy;
            float n = rotationAxis.length();
            if(n > 1e-5) {
                rotationAxis.normalize();
                float radians = n / width() * M_PI;
                // Rotate eye about center around rotation axis.
                float a = cos(radians / 2);
                float s = -sin(radians / 2);
                float b = rotationAxis.x() * s;
                float c = rotationAxis.y() * s;
                float d = rotationAxis.z() * s;
              //  qDebug()<<" "<<b<<" "<<c<<" "<<d<<" ";
                float rotation[9] = {
                    a*a+b*b-c*c-d*d,     2*(b*c-a*d),     2*(b*d+a*c),
                    2*(b*c+a*d), a*a+c*c-b*b-d*d,     2*(c*d-a*b),
                    2*(b*d-a*c),     2*(c*d+a*b), a*a+d*d-b*b-c*c
                };
                camera.eye -= camera.center; // Shift center to origin so can rotate eye about axis through the origin.
                // Rotate eye around rotation axis.
                float ex = camera.eye.x() * rotation[0] + camera.eye.y() * rotation[1] + camera.eye.z() * rotation[2];
                float ey = camera.eye.x() * rotation[3] + camera.eye.y() * rotation[4] + camera.eye.z() * rotation[5];
                float ez = camera.eye.x() * rotation[6] + camera.eye.y() * rotation[7] + camera.eye.z() * rotation[8];
                camera.eye = QVector3D(ex, ey, ez);
                 qDebug()<<" "<<ex<<" "<<ey<<" "<<ez<<" ";
                camera.eye += camera.center; // shift back to center.
                repaint();
                return;
            }
        } else if(pan) {
            float zoom = camera.view().length();
            QVector3D translation = xhat * (dx / width() * zoom) + yhat * (dy / height() * zoom);
            camera.center -= translation;
           camera.eye -= translation;
            repaint();
            return;
        }
    } // rotate || pan
    QOpenGLWidget::mouseMoveEvent(event);
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{

    float degrees = event->angleDelta().y() / 8;
    float steps = degrees / 15;
    if(steps == 0) return;
    if(swapMouseWheelZoomDirection()) {
        steps = -steps;
    }
    float zoom = camera.view().length();
    zoom -= zoom * steps * mouseWheelSensitivity();
    if(zoom < 1e-5) {
        zoom = 1e-5;
    }
    camera.zoom(zoom);
    repaint();
}

void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        selectObject(event->pos());
        if(_selectedObject) {
            editSelectedObject(event->pos());
            return;
        }
    }
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void OpenGLWidget::add_subwidget(OpenGLWidget opglw){
    renderwidgets.push_back(&opglw);
}


  OpenGLWidget::OpenGLWidget(std::vector<RenderPoint> &pointcloud2,int camID )
    {


camID_gl=camID;
pointcloud = pointcloud2;
if(camID!=0){
    _is3D=false;
        if(camID==2){
          //  RotateF(1.0,0.0);
             camera.eye = QVector3D(0.0,-90.0, 0.5);

    }
    else if(camID==3){
        //   camera.eye = QVector3D(-40.04, 42.064, 0.73);
             camera.eye = QVector3D(70.0, 72.064, -30.04);
        }

}
else{ setFocusPolicy(Qt::StrongFocus);
}




//auto  tempvect
pointcloud.resize(100000);



qDebug()<<pointcloud.size();
RenderPoint rp = pointcloud[1];
qDebug()<<rp.r<<" "<<rp.g<<" "<<rp.b;

//int lD = loadData();

// pointcloud = getPointCloudFromApcfCloud(apcf_cloud).makeShared();
}
