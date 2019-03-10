#include "definitionholder.h"

#include <QtCore/QString>
#include <QtCore/QDate>


//-----------------------------------------------------------------------------
// Static variables initialization
//-----------------------------------------------------------------------------

const QString DefinitionHolder::VERSION = "1.1";
const QString DefinitionHolder::NAME = "IOTAcooler";
const QString DefinitionHolder::ORG = "iotacooler.github.io";
const QString DefinitionHolder::UPDATE_URL = "https://raw.githubusercontent.com/joshirio/iota-cooler/master/doc/update/raw_check/latest";
const QString DefinitionHolder::DOWNLOAD_URL = "https://github.com/joshirio/iota-cooler/blob/master/doc/update/update.md";
const QString DefinitionHolder::HELP_URL = "https://github.com/joshirio/iota-cooler/wiki";
const int DefinitionHolder::SOFTWARE_BUILD = 2;
const bool DefinitionHolder::APP_STORE = false;
const bool DefinitionHolder::APPIMAGE_LINUX = false;
const bool DefinitionHolder::DEB_LINUX = false;
const bool DefinitionHolder::WIN_PORTABLE = false;
const bool DefinitionHolder::SNAP_PKG = false;
const QString DefinitionHolder::COPYRIGHT =
        QString("Copyright &copy; 2018-%1 Oirio Joshi")
        .arg(QDate::currentDate().toString("yyyy"));
const QString DefinitionHolder::DEFAULT_NODE = "https://nodes.thetangle.org:443";
const quint32 DefinitionHolder::WALLET_VERSION = 1;
const QString DefinitionHolder::DONATE_URL = "https://github.com/joshirio/iota-cooler/blob/master/doc/donate.md";
