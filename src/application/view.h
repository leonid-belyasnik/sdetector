/**
* \file		view.h
* \brief	Main widget.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef VIEW_H
#define VIEW_H

#include <QDebug>
#include <QtGui>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include "process.h"
#include "files_processor.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QListWidget;
class QTableWidgetItem;
class QProgressBar;
QT_END_NAMESPACE

class ViewMain : public QWidget
{
	Q_OBJECT

public:
	ViewMain(QWidget *parent = 0);

	private slots:
	void browse();
	void find();
	void abort();
	void animateFindClick();

private:
	QComboBox *createComboBox(const QString &text = QString());
	void createFilesTable();
	void createErrorsTable();
	void customEvent(QEvent *ev);

	QComboBox* directoryComboBox;
	QLabel* filesFoundLabel;
	QPushButton* findButton;
	QPushButton* abortButton;
	QTableWidget* filesTable;
	QListWidget* errorsList;
	QProgressBar* progressBar;
	QDir currentDir;
	T1::FilesProcessor Processor;	///< file handler

signals:
	void status(const QString& stat_msg);
public:
	void set_status(const QString& stat_msg) 
	{
		emit status(stat_msg);
	}	
};

#endif // VIEW_H
