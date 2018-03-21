/**
  * \class UpdateManager
  * \brief This class handles software version checking and update info messages
  * \author Oirio Joshi
  * \date 2018-03-21
  */

#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QtCore/QObject>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateManager : public QObject
{
    Q_OBJECT

public:
    UpdateManager(QObject *parent = 0);
    ~UpdateManager();

    /** Start checking for updates */
    void checkForUpdates();

signals:
    void noUpdateSignal();
    void updateErrorSignal();
    void updatesAccepted();

private slots:
    void updateResponseSlot(QNetworkReply*);

private:
    QNetworkAccessManager *m_accessManager;
};

#endif // UPDATEMANAGER_H
