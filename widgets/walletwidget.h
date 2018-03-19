/**
  * \class WalletWidget
  * \brief Stacked widget for main walllet view and other related actions
  * like addresses list and other.
  * \author Oirio Joshi
  * \date 2018-03-19
  */

#ifndef WALLETWIDGET_H
#define WALLETWIDGET_H

#include <QWidget>

#include "../components/abstracttangleapi.h"

namespace Ui {
class WalletWidget;
}

class WalletManager;
class SettingsManager;

class WalletWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WalletWidget(QWidget *parent = 0);
    ~WalletWidget();

    /**
     * @brief Set the current wallet file path
     * and initialize wallet status display
     * @param walletFilePath - wallet file
     */
    void setCurrentWalletPath(const QString &walletFilePath);

signals:
    /**
     * @brief Emitted on wallet close button click
     */
    void walletClosed();

    /** Signal main window to show a status bar message */
    void showStatusMessage(const QString &message);

private slots:
    /**
     * @brief Update balance of the main address
     */
    void updateBalance();
    void closeWalletButtonClicked();
    void copyCurrentAddress();
    void requestFinished(AbstractTangleAPI::RequestType request,
                         const QString &responseMessage);
    void requestError(AbstractTangleAPI::RequestType request,
                      const QString &errorMessage);

private:
    /**
     * @brief Periodically check main address balance
     */
    void startBalanceRefresher();
    void stopBalanceRefresher();

    Ui::WalletWidget *ui;
    QString m_currentWalletFilePath;
    WalletManager *m_walletManager;
    AbstractTangleAPI *m_tangleAPI;
    QTimer *m_BalanceRefreshTimer;
    SettingsManager *m_settingsManager;
};

#endif // WALLETWIDGET_H
