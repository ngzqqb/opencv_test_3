#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <QtWidgets/QtWidgets>
#include "ImageTool.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_imageFileNameButton_clicked() {
    auto varAns = QFileDialog::getOpenFileName(nullptr, {}, ui->imageFileName->text());
    if (varAns.isEmpty()) {
        return;
    }
    ui->imageFileName->setText(varAns);
}

void MainWindow::on_callButton_clicked() {
    auto varFileName = ui->imageFileName->text();
    auto varAngle = sstd::evalAngle(varFileName);
    sstd::saveImage(
    sstd::rotateExternImage(varFileName, varAngle>90?(varAngle-180):varAngle), 
        varFileName+QStringLiteral(".1.bmp"));
}

