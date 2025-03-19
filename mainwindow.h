#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QMenuBar>
#include <QLineEdit>
#include <QPushButton>
#include <vector>
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    void createMenu();
    ~MainWindow();

};

#endif // MAINWINDOW_H
