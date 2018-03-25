#include "settingsmanager.h"
#include "../utils/definitionholder.h"

#include <QtCore/QSettings>

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
    if (DefinitionHolder::WIN_PORTABLE) {
        m_settings = new QSettings("portable_data/settings.ini", QSettings::IniFormat);
    } else {
        m_settings = new QSettings();
    }
}

SettingsManager::~SettingsManager()
{
    delete m_settings;
}

void SettingsManager::saveProperty(const QString &propertyName, const QString &objectName,
                                   const QVariant &value)
{
    m_settings->beginGroup(objectName);
    m_settings->setValue(propertyName, value);
    m_settings->endGroup();
}


QVariant SettingsManager::restoreProperty(const QString &propertyName, const QString &objectName) const
{
    QVariant v;

    m_settings->beginGroup(objectName);
    v = m_settings->value(propertyName);
    m_settings->endGroup();

    return v;
}

void SettingsManager::setDefaultIOTANodeUrl(const QString &url)
{
    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    m_settings->setValue("nodeUrl", url);
    m_settings->endGroup();
}

QString SettingsManager::getDefaultIOTANodeUrl() const
{
    QString url;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    url = m_settings->value("nodeUrl", DefinitionHolder::DEFAULT_NODE).toString();
    m_settings->endGroup();

    return url;
}

UtilsIOTA::DeviceRole SettingsManager::getDeviceRole() const
{
    UtilsIOTA::DeviceRole role;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    role = (UtilsIOTA::DeviceRole) m_settings->value("deviceRole", 0).toInt();
    m_settings->endGroup();

    return role;
}

void SettingsManager::setDeviceRole(UtilsIOTA::DeviceRole role)
{
    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    m_settings->setValue("deviceRole", (int) role);
    m_settings->endGroup();
}

void SettingsManager::setCheckUpdates(bool b)
{
    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    m_settings->setValue("checkUpdates", b);
    m_settings->endGroup();
}

bool SettingsManager::getCheckUpdates() const
{
    bool b;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    b = m_settings->value("checkUpdates", true).toBool();
    m_settings->endGroup();

    return b;
}

void SettingsManager::saveGeometry(const QString &objectName,
                                   const QByteArray &geometry)
{
    m_settings->beginGroup(objectName);
    m_settings->setValue("geometry", geometry);
    m_settings->endGroup();
}

QByteArray SettingsManager::restoreGeometry(const QString &objectName) const
{
    QByteArray g;

    m_settings->beginGroup(objectName);
    g = m_settings->value("geometry").toByteArray();
    m_settings->endGroup();

    return g;
}

void SettingsManager::saveState(const QString &objectName, const QByteArray &state)
{
    m_settings->beginGroup(objectName);
    m_settings->setValue("state", state);
    m_settings->endGroup();
}

QByteArray SettingsManager::restoreState(const QString &objectName) const
{
    QByteArray s;

    m_settings->beginGroup(objectName);
    s = m_settings->value("state").toByteArray();
    m_settings->endGroup();

    return s;
}

bool SettingsManager::isClipboardGuardEnabled()
{
    bool b;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    b = m_settings->value("cGuard", true).toBool();
    m_settings->endGroup();

    return b;
}

void SettingsManager::setClipboardGuardEnabled(bool b)
{
    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    m_settings->setValue("cGuard", b);
    m_settings->endGroup();
}
