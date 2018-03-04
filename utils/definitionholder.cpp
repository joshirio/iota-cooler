#include "definitionholder.h"

#include <QtCore/QString>
#include <QtCore/QDate>


//-----------------------------------------------------------------------------
// Static variables initialization
//-----------------------------------------------------------------------------

QString DefinitionHolder::VERSION = "1.0";
QString DefinitionHolder::NAME = "IOTAcooler";
QString DefinitionHolder::ORG = "iota-cooler";
QString DefinitionHolder::UPDATE_URL = "https://joshirio.github.io/iotacooler/update/iotacooler/updates";
QString DefinitionHolder::DOWNLOAD_URL = "https://joshirio.github.io/iotacooler/update.html";
QString DefinitionHolder::HELP_URL = "https://github.com/joshirio/iota-cooler/wiki";
int DefinitionHolder::SOFTWARE_BUILD = 1;
bool DefinitionHolder::APP_STORE = false;
bool DefinitionHolder::APPIMAGE_LINUX = false;
bool DefinitionHolder::WIN_PORTABLE = false;
QString DefinitionHolder::COPYRIGHT =
        QString("Copyright &copy; 2018-%1 Oirio Joshi")
        .arg(QDate::currentDate().toString("yyyy"));

