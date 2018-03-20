/**
  * \class MultisigTransferWidget
  * \brief Stacked widget with step-by-step instructions
  * for multisig transfer creation, with online and offline tasks.
  * \author Oirio Joshi
  * \date 2018-03-20
  */

#ifndef MULTISIGTRANSFERWIDGET_H
#define MULTISIGTRANSFERWIDGET_H

#include <QWidget>
#include "../components/abstracttangleapi.h"

class WalletManager;
class QLabel;
class QLineEdit;
class QHBoxLayout;

namespace Ui {
class MultisigTransferWidget;
}

class MultisigTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MultisigTransferWidget(QWidget *parent = 0);
    ~MultisigTransferWidget();

    /** Setup UI for new transfer */
    void prepareNewTransfer(const QString &walletPath);

    /** Prepare UI about next offline step from online signer */
    void showContinueWithOfflineSigner(const QString &walletPath);

    /** Prepare UI about next online step from online signer */
    void showContinueWithOnlineSigner(const QString &walletPath);

    /** Prepare UI for offline transaction signing */
    void prepareColdTransferSign(const QString &walletPath);

    /** Prepare UI for online transaction signing and broadcasting */
    void prepareHotTransferSign(const QString &walletPath);

signals:
    /** Unfinished transfer was aborted */
    void transferCancelled();

private slots:
    void nextPage();
    void addReceiverButtonClicked();
    void removeReceiverButtonClicked();
    void receiversNextButtonClicked();
    void onlineSignQuitButtonClicked();
    void abortCurrentTransaction();
    void offlineSignConfirmButtonClicked();
    void txFinalQuitButtonClicked();
    void txFinalNextButtonClicked();
    void updateBalance();
    void requestFinished(AbstractTangleAPI::RequestType request,
                         const QString &responseMessage);
    void requestError(AbstractTangleAPI::RequestType request,
                      const QString &errorMessage);

private:
    void clearAllReceivers();

    Ui::MultisigTransferWidget *ui;
    WalletManager *m_walletManager;
    AbstractTangleAPI *m_tangleAPI;
    QHash<QString, QString> m_receiversMap; //address, amount
    QList<QLabel*> m_receiversLabelList;
    QList<QLineEdit*> m_receiversAddressLineList;
    QList<QLineEdit*> m_receiversAmountLineList;
    QList<QHBoxLayout*> m_receiversLayoutList;
    QString m_currentWalletPath;
    QStringList m_receiverList;
    QStringList m_amountList;
    QString m_walletBalance;
};

#endif // MULTISIGTRANSFERWIDGET_H
