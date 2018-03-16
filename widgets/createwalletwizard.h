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
    void walletError(const QString &message);

private:
    Ui::CreateWalletWizard *ui;
    WalletManager *m_walletManager;
};

#endif // CREATEWALLETWIZARD_H
