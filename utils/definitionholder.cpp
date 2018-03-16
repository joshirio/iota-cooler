#include "definitionholder.h"

#include <QtCore/QString>
#include <QtCore/QDate>


//-----------------------------------------------------------------------------
// Static variables initialization
//-----------------------------------------------------------------------------

const QString DefinitionHolder::VERSION = "1.0";
const QString DefinitionHolder::NAME = "IOTAcooler";
const QString DefinitionHolder::ORG = "iotacooler.github.io";
const QString DefinitionHolder::UPDATE_URL = "https://joshirio.github.io/iotacooler/update/iotacooler/updates";
const QString DefinitionHolder::DOWNLOAD_URL = "https://joshirio.github.io/iotacooler/update.html";
const QString DefinitionHolder::HELP_URL = "https://github.com/joshirio/iota-cooler/wiki";
const int DefinitionHolder::SOFTWARE_BUILD = 1;
const bool DefinitionHolder::APP_STORE = false;
const bool DefinitionHolder::APPIMAGE_LINUX = false;
const bool DefinitionHolder::WIN_PORTABLE = false;
const QString DefinitionHolder::COPYRIGHT =
        QString("Copyright &copy; 2018-%1 Oirio Joshi")
        .arg(QDate::currentDate().toString("yyyy"));
const QString DefinitionHolder::DEFAULT_NODE = "https://field.carriota.com:443";
const quint32 DefinitionHolder::WALLET_VERSION = 1;
const QString DefinitionHolder::DONATE_URL = "https://github.com/joshirio/iota-cooler/blob/master/doc/donate.md";
