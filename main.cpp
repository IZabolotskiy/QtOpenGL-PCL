

// std
#include <iostream>
#include <thread>
#include <iomanip>
#include <chrono>



#include "openglwidget.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QMainWindow>
//#include "pointviewer.h"
#include "mainwindow.h"

#include <QList>
#undef emit

/*
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    RPointViewer viewer;
    viewer.show();

    return app.exec();
}
*/

int _wres=1280,_hres=720;
std::vector<RenderPoint> pointcloud;



int main(int argc, char *argv[])
{
    /*
    QSurfaceFormat format;
    format.setVersion(3,3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
*/
    QList<OpenGLWidget*> widgets_c;

    //widget
    QApplication a(argc, argv);

    // QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    MainWindow mainWindow;

    OpenGLWidget *widget1 = new OpenGLWidget(pointcloud,0);
  OpenGLWidget *widget2 = new OpenGLWidget(pointcloud,1);
   OpenGLWidget *widget3 = new OpenGLWidget(pointcloud,2);
   OpenGLWidget *widget4 = new OpenGLWidget(pointcloud,3);

   widget1->renderwidgets.push_back(widget2);
    widget1->renderwidgets.push_back(widget3);
     widget1->renderwidgets.push_back(widget4);

/*
   widget1->add_subwidget(widget2);
    widget1->add_subwidget(widget3);
    widget1->add_subwidget(widget4);
    */

   /*
    widgets_c.append(widget1);
      widgets_c.append(widget2);
        widgets_c.append(widget3);
          widgets_c.append(widget4);
*/

    /*
    qobject_cast< OpenGLWidget*>(widget1)->add_subwidget(widget2);
qobject_cast< OpenGLWidget*>(widget1)->add_subwidget(widget3);
qobject_cast< OpenGLWidget*>(widget1)->add_subwidget(widget4);
*/
    QLineEdit *input_begin = new QLineEdit();
    QLineEdit *input_end = new QLineEdit();
    QPushButton *button_goto = new QPushButton("goto");
    QPushButton *button_prev= new QPushButton("prev");
    QPushButton *button_next = new QPushButton("next");

    QHBoxLayout* buttons= new QHBoxLayout;
    /*
    buttons->addWidget(input_begin,15);
    buttons->addWidget(input_end,15);
    buttons->addWidget(button_goto,20);
    buttons->addWidget(button_next,10);
    buttons->addWidget(button_prev,10);
*/

    QVBoxLayout* leftL = new QVBoxLayout;

    leftL->addWidget(widget1);

    QVBoxLayout* rightL = new QVBoxLayout;
    rightL->addWidget(widget2);
    rightL->addWidget(widget3);
    // rightL->addWidget(widget4);

    QHBoxLayout* mainL = new QHBoxLayout;
    mainL->addLayout(leftL,65);
    mainL->addLayout(rightL,35);
    // mainL->addLayout(leftL);
    //  mainL->addLayout(rightL);

    QVBoxLayout* lastVertical =  new QVBoxLayout;

    //  lastVertical->addLayout(mainL);
    // lastVertical->addWidget(widget4);

    lastVertical->addLayout(buttons,8);
    lastVertical->addLayout(mainL,67);
    lastVertical->addWidget(widget4,35);

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout( lastVertical);

    //connects for buttons

 //   connect(button_goto, &QPushButton::clicked, this, &widget1::reloadsomepoints);


    mainWindow.setCentralWidget(centralWidget);
    mainWindow.setWindowTitle("Pointcloud QtViewer ver0.53");
    mainWindow.resize(_wres,_hres);
    mainWindow.createMenu();
    mainWindow.show();
    return a.exec();
}
