#include "widgets/mainwindow.h"
#include "utils/definitionholder.h"
#include "utils/qtsingleapplication/qtsingleapplication.h"

#include <QtCore/QTranslator>
#include <QtCore/QLocale>


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //enable high DPI scaling on Windows and Linux
    //NOTE: don't move, must be called before QApplication initialization
    //or it won't work at all
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QtSingleApplication IOTAcoolerApp(argc, argv);
    IOTAcoolerApp.setApplicationName(DefinitionHolder::NAME);
    IOTAcoolerApp.setApplicationVersion(DefinitionHolder::VERSION);
    IOTAcoolerApp.setOrganizationName(DefinitionHolder::NAME);
    IOTAcoolerApp.setOrganizationDomain(DefinitionHolder::ORG);
    IOTAcoolerApp.setWindowIcon(QIcon(":/images/icons/iotacooler.png"));

    //only one instance allowed
    if (IOTAcoolerApp.sendMessage("Wake up!"))
        return 0;

    //setup translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      ":/languages");
    IOTAcoolerApp.installTranslator(&qtTranslator);
    QTranslator myappTranslator;
    myappTranslator.load("iotacooler_" + QLocale::system().name(), ":/languages");
    IOTAcoolerApp.installTranslator(&myappTranslator);

    //init gui
    MainWindow w;
    w.show();

    //TODO: handle args for wallet file open and add mimetype with icon
    //if wallet is already open, close it first (mainwindow's job)

    //wake up window
    IOTAcoolerApp.setActivationWindow(&w);

    return IOTAcoolerApp.exec();
}

