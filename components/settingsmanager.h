/**
  * \class SettingsManager
  * \brief This class manages all settings related task, like storing/restoring
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>

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
    QString getDefaultIOTANodeUrl();

    /**
     * @brief Set the default url for IOTA IRI node
     * @param url - node url (protocol://host:port)
     */
    void setDefaultIOTANodeUrl(const QString &url);

private:
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
