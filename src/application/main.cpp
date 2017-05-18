#include <QApplication>
#include "mainwindow.h"
#include "config.h"

#ifdef _USE_VLD
#include <vld.h>
#endif

int main(int argc, char *argv[])
{
	QStringList paths = QCoreApplication::libraryPaths();
	paths.append(".");
	paths.append("platforms");
	QCoreApplication::setLibraryPaths(paths);

    QApplication app(argc, argv);
    MainWindow win;
    win.show();
	
    return app.exec();
}
