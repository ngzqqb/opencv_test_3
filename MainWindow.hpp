﻿#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_imageFileNameButton_clicked();

    void on_callButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_HPP
