/**
* \file		mainwindow.h
* \brief	Main application window.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "view.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

	ViewMain *wCenter;
public:
	explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void SetStatus(const QString& stat_msg);

};


#endif // MAINWINDOW_H
