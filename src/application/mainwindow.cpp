#include "mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle(QObject::tr("GUI application"));
    this->setMinimumSize(675,225);
 
    wCenter = new ViewMain(this);
    this->setCentralWidget(wCenter);

	connect(wCenter, &ViewMain::status,  this, &MainWindow::SetStatus);
	SetStatus(QString("Started"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::SetStatus(const QString& stat_msg)
{
	statusBar()->showMessage(stat_msg);
}

