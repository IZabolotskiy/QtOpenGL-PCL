#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenu() {
    // Create a menu bar
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
       // QHBoxLayout* mainL = new QHBoxLayout;
    // Create a top-level menu
    QMenu *fileMenu = menuBar->addMenu(tr("File"));

    // Create a top-level menu 2
    QMenu *optionsMenu = menuBar->addMenu(tr("Options"));

    // Create a top-level menu 7
    QMenu *helpMenu = menuBar->addMenu(tr("Help"));

    // Create a top-level menu 8
    QMenu *infoMenu = menuBar->addMenu(tr("Info"));


    // Add actions (buttons) to the menu
    QAction *newAction = new QAction(tr("New"), this);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(tr("Open"), this);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction(tr("Save"), this);
    fileMenu->addAction(saveAction);

    QAction *exitAction = new QAction(tr("Quit"), this);
    fileMenu->addAction(exitAction);

    QAction *infoAction = new QAction(tr("Info"), this);
    infoMenu->addAction(infoAction);

    QAction *helpAction = new QAction(tr("Help"), this);
    helpMenu->addAction(helpAction);

    // Connect actions to slots (optional)
    /*
      connect(newAction, &QAction::triggered, this, &MainWindow::newButtonClicked);
      connect(openAction, &QAction::triggered, this, &MainWindow::openButtonClicked);
      connect(saveAction, &QAction::triggered, this, &MainWindow::saveButtonClicked);
      connect(exitAction, &QAction::triggered, this, &MainWindow::exitButtonClicked);
      connect(infoAction, &QAction::triggered, this, &MainWindow::infoButtonClicked);
      connect(helpAction, &QAction::triggered, this, &MainWindow::helpButtonClicked);
*/


}
