/**
  * \class RestoreWalletWidget
  * \brief Stacked widget for funds recovery from seeds
  * \author Oirio Joshi
  * \date 2018-03-30
  */

#ifndef RESTOREWALLETWIDGET_H
#define RESTOREWALLETWIDGET_H

#include <QWidget>
#include "../components/abstracttangleapi.h"

namespace Ui {
class RestoreWalletWidget;
}

class WalletManager;

class RestoreWalletWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RestoreWalletWidget(QWidget *parent = 0);
    ~RestoreWalletWidget();

    /** Prepare UI about next offline step from online signer */
    void showContinueWithOfflineSigner(const QString &walletPath);

    /** Prepare UI about next online step from online signer */
    void showContinueWithOnlineSigner(const QString &walletPath);

    /** Prepare UI about next offline transfer signing step */
    void showContinueWithRecoveryColdSign(const QString &walletPath);

    /** Prepare UI about next online transfer sending step */
    void showContinueWithRecoveryHotSend(const QString &walletPath);

    /** Prepare UI for offline wallet recovery */
    void prepareColdRecovery(const QString &walletPath);

    /** Prepare UI for online wallet recovery info */
    void prepareHotRecoveryInfo(const QString &walletPath);

    /** Prepare UI for online wallet recovery and tx broadcasting */
    void prepareHotRecovery(const QString &walletPath);

    /** Prepare UI for offline recovery transfer signing */
    void prepareRecoveryColdSign(const QString &walletPath);

    /** Prepare UI for online recovery transfer sending */
    void prepareRecoveryHotSend(const QString &walletPath);

signals:
    /** Emitted when this wizard is cancelled */
    void restoreWalletCancelled();

    /** Emitted when this wizard is completed */
    void restoreWalletCompleted();

private slots:
    void quitButtonClicked();
    void restoreInfoNextButtonClicked();
    void offlineSeedsNextButtonClicked();
    void cancelGenAddrButtonClicked();
    void nextGenAddrButtonClicked();
    void checkAddrNextButtonClicked();
    void checkAddrCancelButtonClicked();
    void recoverSignSeedsCancelButtonClicked();
    void recoveryColdSignNextButtonClicked();
    void recoverSignSeedsNextButtonClicked();
    void requestFinished(AbstractTangleAPI::RequestType request,
                         const QString &responseMessage);
    void requestError(AbstractTangleAPI::RequestType request,
                      const QString &errorMessage);
    void addressGenerationProgress(int currentIndex, int endIndex);

private:
    void nextPage();
    void stopRequestAndCancelRecovery();
    void recoverFundsIsAddrSpentRequest();

    Ui::RestoreWalletWidget *ui;
    QString m_currentWalletPath;
    QStringList m_currentRecoveryStepResults;
    AbstractTangleAPI *m_tangleAPI;
    WalletManager *m_walletManager;
    int m_startIndex;
    int m_endIndex;
    int m_currentIndex;
};

#endif // RESTOREWALLETWIDGET_H
