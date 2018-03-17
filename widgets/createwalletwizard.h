/**
  * \class CreateWalletWizard
  * \brief Stacked widget with step-by-step instructions
  * for cold wallet creation, with online and offline tasks.
  * \author Oirio Joshi
  * \date 2018-03-13
  */

#ifndef CREATEWALLETWIZARD_H
#define CREATEWALLETWIZARD_H

#include <QWidget>
#include "../components/abstracttangleapi.h"

class WalletManager;

namespace Ui {
class CreateWalletWizard;
}

class CreateWalletWizard : public QWidget
{
    Q_OBJECT

public:
    explicit CreateWalletWizard(QWidget *parent = 0);
    ~CreateWalletWizard();

    /** Set the current view to wallet offline init.
     * If the device role is online, show info about
     * continuing with offline device as next step
     */
    void setOfflineWalletInitStep();

signals:
    /** Emitted when user cancels the wizard */
    void walletCreationCancelled();

private slots:
    void infoNextButtonClicked();
    void infoCancelButtonClicked();
    void wConfBrowseButtonClicked();
    void wConfUpdateNextButtonState();
    void wConfNextButtonClicked();
    void wInitOnlineQuitButtonClicked();
    void offlineInitWalletButtonClicked();
    void walletError(const QString &message);
    void walletParseError(const QString &message);
    void walletPassError();
    void tangleAPIRequestFinished(AbstractTangleAPI::RequestType request,
                                  const QString &response);
    void tangleAPIRequestError(AbstractTangleAPI::RequestType request,
                               const QString &errorMessage);

private:
    Ui::CreateWalletWizard *ui;
    WalletManager *m_walletManager;
    AbstractTangleAPI *m_tangleAPI;
    QStringList m_walletInitStepResults;
};

#endif // CREATEWALLETWIZARD_H
