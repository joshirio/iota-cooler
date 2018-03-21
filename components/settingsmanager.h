/**
  * \class SettingsManager
  * \brief This class manages all settings related task, like storing/restoring
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include "../utils/utilsiota.h"

class QSettings;

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();

    /** Save the state of a property
      * @param propertyName - the key name
      * @param objectName - the object that holds the property
      * @param value - the value to store
      */
    void saveProperty(const QString& propertyName, const QString& objectName,
                      const QVariant& value);

    /** Restore the state of a property
      * @param propertyName - the key name
      * @param objectName - the object that holds the property
      * @return the stored value
      */
    QVariant restoreProperty(const QString& propertyName,
                             const QString& objectName) const;

    /**
     * @brief Get the default url for IOTA IRI node
     * @return QString - node url (protocol://host:port)
     */
    QString getDefaultIOTANodeUrl() const;

    /**
     * @brief Set the default url for IOTA IRI node
     * @param url - node url (protocol://host:port)
     */
    void setDefaultIOTANodeUrl(const QString &url);

    /**
     * @brief Get current role for the device
     * @return UtilsIOTA::DeviceRole -  role (enum)
     */
    UtilsIOTA::DeviceRole getDeviceRole() const;

    /**
     * @brief Set current device role
     * @param role - UtilsIOTA::DeviceRole (enum)
     */
    void setDeviceRole(UtilsIOTA::DeviceRole role);

    /** Save state on check updates at startup */
    void setCheckUpdates(bool b);

    /** Check updates automatically at startup */
    bool getCheckUpdates() const;

    /** Save the geometry property for the given object name */
    void saveGeometry(const QString& objectName, const QByteArray& geometry);

    /** Restore the geometry property for the specified object */
    QByteArray restoreGeometry(const QString& objectName) const;

    /**
      * Save the state property (dock, toolbars, etc...)
      * for the given object name
      */
    void saveState(const QString& objectName, const QByteArray& state);

    /** Restore the state property for the specified object */
    QByteArray restoreState(const QString& objectName) const;

private:
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
