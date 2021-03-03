#include "../../Kernel/src/Version.h"
#include "../../UserInterface/src/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(Version::getOrganisationName());
    QCoreApplication::setApplicationName(Version::getApplicationName());

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
