#include "mainwindow.h"
#include <QtWidgets>

using namespace T1;

enum { absoluteFileNameRole = Qt::UserRole + 1 };

ViewMain::ViewMain(QWidget *parent)
	: QWidget(parent), Processor(this)
{
	QPushButton *browseButton = new QPushButton(tr("Browse..."), this);
	connect(browseButton, &QAbstractButton::clicked, this, &ViewMain::browse);
	findButton = new QPushButton(tr("&Find >>>"), this);
	connect(findButton, &QAbstractButton::clicked, this, &ViewMain::find);

	abortButton = new QPushButton(tr("Abort"), this);
	connect(abortButton, &QAbstractButton::clicked, this, &ViewMain::abort);

	directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
	connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
		this, &ViewMain::animateFindClick);

	filesFoundLabel = new QLabel;

	createFilesTable();
	createErrorsTable();
	QSplitter * splitter(new QSplitter(this));
	QVBoxLayout* pVSplitLayout = new QVBoxLayout(splitter);
	pVSplitLayout->addWidget(filesTable, 0, Qt::AlignCenter);
	pVSplitLayout->addWidget(errorsList, 1, Qt::AlignBottom);
	splitter->setLayout(pVSplitLayout);
	splitter->setOrientation(Qt::Vertical);
	splitter->setStretchFactor(0,3);
	splitter->setStretchFactor(1,1);

	progressBar = new QProgressBar(this);

	progressBar->setMaximumHeight(50);
	progressBar->setRange(0, 0);
	progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	progressBar->setTextVisible(false);
	progressBar->hide();

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->addWidget(directoryComboBox, 0, 0);
	mainLayout->addWidget(browseButton, 0, 1);
	mainLayout->addWidget(findButton, 0, 2);
	mainLayout->addWidget(abortButton, 0, 3);
	mainLayout->addWidget(splitter, 1, 0, 1, 4);
	mainLayout->addWidget(filesFoundLabel, 2, 0, 1, 4);
	mainLayout->addWidget(progressBar, 3, 0, 1, 4);
}

void ViewMain::customEvent(QEvent *ev)
{
	if ((int)ev->type() == ProcessEvent::ProcessType) 
	{
		ProcessEvent* pe = dynamic_cast<ProcessEvent*>(ev);
		EvMode m = pe->mode();
		switch (m)
		{
		case EvMode::Start:
		{
			if (progressBar->isVisible())
				progressBar->hide();

			filesTable->setRowCount(0);
			progressBar->setRange(0, pe->value());
			progressBar->show();
			set_status(pe->status());
			set_status(tr("Found %1 file(s). Processing ...").arg(pe->value()));
			findButton->setDisabled(true);
		}
		break;
		case EvMode::Stop:
		{
			progressBar->setValue(progressBar->value() + 1);
			set_status(tr("Processed %1 file(s). Found %2 threats.").arg(progressBar->maximum()).arg(filesTable->rowCount()));
			progressBar->hide();
			progressBar->setRange(0, 0);
			findButton->setDisabled(false);
		}
		break;
		case EvMode::Abort:
		{
			progressBar->setValue(progressBar->value() + 1);
			set_status(tr("Aborted! Processed %1 file(s). Found %2 threats.").arg(progressBar->value()).arg(filesTable->rowCount()));
			progressBar->hide();
			progressBar->setRange(0, 0);
			findButton->setDisabled(false);
			abortButton->setDisabled(false);
		}
		break;
		case EvMode::Max:
		{
			progressBar->setMaximum(pe->value());
		}
		break;
		case EvMode::Inc:
		{
			progressBar->setValue(progressBar->value() + 1);
		}
		break;
		case EvMode::Dec:
		{
			progressBar->setValue(progressBar->value() - 1);
		}
		break;
		case EvMode::Status:
		{
			set_status(pe->status());
		}
		break;
		case EvMode::Malware:
		{
			filesTable->setUpdatesEnabled(false);
			int row = filesTable->rowCount();
			filesTable->insertRow(row);
			filesTable->setItem(row, 0, new QTableWidgetItem(pe->file()));
			filesTable->setItem(row, 1, new QTableWidgetItem(pe->guid()));
			filesTable->setItem(row, 2, new QTableWidgetItem(tr("%1").arg(pe->value())));
			filesTable->resizeColumnToContents(1);
			filesTable->setUpdatesEnabled(true);
			
		}
		break;
		case EvMode::Error:
		{
			errorsList->addItem(pe->status());
		}
		break;
		}
	}
	QWidget::customEvent(ev);
	
}

void ViewMain::browse()
{
	QString directory =
		QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Scan Directory"), QDir::currentPath()));

	if (!directory.isEmpty()) {
		if (directoryComboBox->findText(directory) == -1)
			directoryComboBox->addItem(directory);

		directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
	}
}

static void updateComboBox(QComboBox *comboBox)
{
	if (comboBox->findText(comboBox->currentText()) == -1)
		comboBox->addItem(comboBox->currentText());
}

void ViewMain::find()
{
	filesTable->setRowCount(0);
	errorsList->clear();
	QString path = QDir::cleanPath(directoryComboBox->currentText());
	updateComboBox(directoryComboBox);
	currentDir = QDir(path);
	if (Processor.run(path))
		findButton->setDisabled(true);
}

void ViewMain::abort()
{
	if (Processor.is_running())
	{
		Processor.abort();
		abortButton->setDisabled(true);
		set_status(QString("Aborting ..."));
	}
}

void ViewMain::animateFindClick()
{
	findButton->animateClick();
}

QComboBox *ViewMain::createComboBox(const QString &text)
{
	QComboBox *comboBox = new QComboBox;
	comboBox->setEditable(true);
	comboBox->addItem(text);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

void ViewMain::createFilesTable()
{
	filesTable = new QTableWidget(0, 3);
	QHeaderView* hw = filesTable->horizontalHeader();
	hw->setSectionResizeMode(0, QHeaderView::Stretch);
	filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	filesTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	filesTable->setHorizontalHeaderLabels(QString("Filename;GUID;Pos").split(";"));
	filesTable->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
	filesTable->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
	filesTable->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
	filesTable->verticalHeader()->hide();
	filesTable->setShowGrid(false);
}

void ViewMain::createErrorsTable()
{
	errorsList = new QListWidget();
	errorsList->setAlternatingRowColors(true);
	errorsList->setStyleSheet("color:red;");
}
