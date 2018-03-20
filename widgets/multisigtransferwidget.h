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
    void prepareNewTransfer();

signals:
    /** Unfinished transfer was aborted */
    void transferCancelled();

private slots:
    void nextPage();
    void addReceiverButtonClicked();
    void removeReceiverButtonClicked();
    void receiversNextButtonClicked();
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
    QHash<QString, QString> m_receiversMap;
    QList<QLabel*> m_receiversLabelList;
    QList<QLineEdit*> m_receiversAddressLineList;
    QList<QLineEdit*> m_receiversAmountLineList;
    QList<QHBoxLayout*> m_receiversLayoutList;
};

#endif // MULTISIGTRANSFERWIDGET_H
