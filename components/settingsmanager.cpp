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

QString SettingsManager::getDefaultIOTANodeUrl()
{
    QString url;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    url = m_settings->value("nodeUrl", DefinitionHolder::DEFAULT_NODE).toString();
    m_settings->endGroup();

    return url;
}
