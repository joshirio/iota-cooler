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
#include "../utils/utilsiota.h"

namespace Ui {
class WalletWidget;
}

class WalletManager;
class SettingsManager;
class QSystemTrayIcon;

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

    /** Emitted on send button (new transaction) clicked */
    void makeNewTransactionSignal();

private slots:
    /**
     * @brief Update balance of the main address
     */
    void updateBalance();
    void updateUnconfirmedBalance();
    void updateCurrentAddrTxHistory();
    void closeWalletButtonClicked();
    void copyCurrentAddress();
    void tangleExplorerButtonClicked();
    void sendButtonClicked();
    void addressesButtonClicked();
    void addressesBackButtonClicked();
    void addressesViewTangleButtonClicked();
    void newTransactionsNotification();
    void requestFinished(AbstractTangleAPI::RequestType request,
                         const QString &responseMessage);
    void requestError(AbstractTangleAPI::RequestType request,
                      const QString &errorMessage);

private:
    void startTangleRefreshers();
    void stopTangleRefreshers();
    void loadPastTxs();
    void checkAddressDirty();

    Ui::WalletWidget *ui;
    QString m_currentWalletFilePath;
    WalletManager *m_walletManager;
    AbstractTangleAPI *m_tangleBalanceCheckAPI;
    AbstractTangleAPI *m_tangleHistoryCheckAPI;
    QTimer *m_tangleRefreshTimer;
    SettingsManager *m_settingsManager;
    QList<UtilsIOTA::Transation> m_incomingTxList;
    QList<UtilsIOTA::Transation> m_currentAddrTxList;
    QList<UtilsIOTA::Transation> m_outgoingTxList;
    quint64 m_lastCheckedBalance;
    QSystemTrayIcon *m_tray;
};

#endif // WALLETWIDGET_H
