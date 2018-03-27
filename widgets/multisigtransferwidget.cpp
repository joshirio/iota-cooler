#include "multisigtransferwidget.h"
#include "ui_multisigtransferwidget.h"
#include "../components/smidgentangleapi.h"
#include "../utils/utilsiota.h"
#include "../components/walletmanager.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QRegExpValidator>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QApplication>

MultisigTransferWidget::MultisigTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultisigTransferWidget)
{
    ui->setupUi(this);
    m_walletManager = &WalletManager::getInstance();
    m_currentWalletPath = "/invalid";

    connect(ui->infoCancelButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::transferCancelled);
    connect(ui->infoNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::nextPage);
    connect(ui->rAddButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::addReceiverButtonClicked);
    connect(ui->rRemoveButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::removeReceiverButtonClicked);
    connect(ui->rCancelButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::transferCancelled);
    connect(ui->rNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::receiversNextButtonClicked);
    connect(ui->onlineSignQuitButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::onlineSignQuitButtonClicked);
    connect(ui->goOfflineAbortTransactionButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::abortCurrentTransaction);
    connect(ui->offlineSignAbortTransactionButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::abortCurrentTransaction);
    connect(ui->offlineSignConfirmButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::offlineSignConfirmButtonClicked);
    connect(ui->offlineSigningProgressAbortTxButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::abortCurrentTransaction);
    connect(ui->offlineSigningTxNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::nextPage);
    connect(ui->broadcastAbortTxButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::abortCurrentTransaction);
    connect(ui->broadcastAbortTxButton_2, &QPushButton::clicked,
            this, &MultisigTransferWidget::abortCurrentTransaction);
    connect(ui->txFinalQuitButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::txFinalQuitButtonClicked);
    connect(ui->txFinalNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::txFinalNextButtonClicked);
    connect(ui->broadcastFinishButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::broadcastFinishButtonClicked);

    //tangle api
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &MultisigTransferWidget::requestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &MultisigTransferWidget::requestError);

    //tag input
    QRegExp tagRe("^[A-Za-z9]{27}$");
    QRegExpValidator *tagValidator = new QRegExpValidator(tagRe, this);
    ui->tagLinEdit->setValidator(tagValidator);
}

MultisigTransferWidget::~MultisigTransferWidget()
{
    delete ui;
}

void MultisigTransferWidget::prepareNewTransfer(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(0);
    clearAllReceivers();
    addReceiverButtonClicked();
    updateBalance();
    ui->infoNextButton->setFocus();
}

void MultisigTransferWidget::showContinueWithOfflineSigner(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(2);
}

void MultisigTransferWidget::showContinueWithOnlineSigner(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(5);
}

void MultisigTransferWidget::prepareColdTransferSign(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    bool error = false;
    QString errorMessage;
    m_receiverList.clear();
    m_amountList.clear();
    m_tag.clear();
    ui->offlineSeedLineEdit->setClearButtonEnabled(true);

    //load tx
    QVariantList opArgs;
    m_walletManager->getCurrentWalletOp(opArgs);
    if (opArgs.size() >= 3) {
    ui->stackedWidget->setCurrentIndex(3);
    m_walletBalance = opArgs.at(0).toString();
    m_receiverList = opArgs.at(1).toStringList();
    m_amountList = opArgs.at(2).toStringList();
    m_tag = opArgs.at(3).toString();

    QString text;
    text.append(tr("<b>Wallet balance (iota):</b> %1 <br />").arg(m_walletBalance));
    quint64 totSpending = 0;
    foreach (QString tsp, m_amountList) {
        totSpending += tsp.toULongLong();
    }
    text.append(tr("<b>Total spending (iota):</b> %1 <br />").arg(totSpending));
    if (m_tag != QString("9").repeated(27))
        text.append(tr("<b>Optional tag:</b> %1 <br />")
                    .arg(UtilsIOTA::getEasyReadableTag(m_tag)));
    for (int i = 0; i < m_receiverList.size(); i++) {
        text.append("<hr />");
        text.append(tr("<b>Receiver %1</b><br />").arg(i+1));
        text.append(tr("<b>Address:</b> %2 <br />").arg(m_receiverList.at(i)));
        text.append(tr("<b>Amount (iota):</b> %2").arg(m_amountList.at(i)));
    }
    ui->confirmTxTextArea->setText(text);

    } else {
        error = true;
        errorMessage = tr("Missing cold signing arguments!");
    }

    if (error) {
        QMessageBox::critical(this, tr("Cold Singing Error"), errorMessage);
        emit transferCancelled();
    }
}

void MultisigTransferWidget::prepareHotTransferSign(const QString &walletPath)
{
    m_currentWalletPath = walletPath;

    //load tx
    QVariantList opArgs;
    m_walletManager->getCurrentWalletOp(opArgs);
    m_walletBalance = opArgs.at(0).toString();
    m_receiverList = opArgs.at(1).toStringList();
    m_amountList = opArgs.at(2).toStringList();
    m_tag = opArgs.at(3).toString();

    ui->stackedWidget->setCurrentIndex(6);
}

void MultisigTransferWidget::nextPage()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()
                                       + 1);
}

void MultisigTransferWidget::addReceiverButtonClicked()
{
    int receiverCount = m_receiversLabelList.size();
    int currentReceiver = receiverCount + 1;

    QRegExp addrRe("^[A-Z9]{90}$");
    QRegExpValidator *addrValidator = new QRegExpValidator(addrRe, this);
    QRegExp amountRe("^[0-9]*$");
    QRegExpValidator *amountValidator = new QRegExpValidator(amountRe, this);

    QLabel *rLabel = new QLabel(tr("Receiver ") +
                                QString::number(currentReceiver) + ":",
                                this);
    QLineEdit *rAddrLine = new QLineEdit(this);
    rAddrLine->setPlaceholderText(tr("Receiver %1 address").arg(currentReceiver));
    rAddrLine->setClearButtonEnabled(true);
    rAddrLine->setValidator(addrValidator);
    QLineEdit *rAmountLine = new QLineEdit(this);
    rAmountLine->setPlaceholderText(tr("Receiver %1 amount (iota)").arg(currentReceiver));
    rAmountLine->setClearButtonEnabled(true);
    rAmountLine->setValidator(amountValidator);
    QHBoxLayout *rLayout = new QHBoxLayout(); //becomes child of scrollAreaLayout (no leak)
    rLayout->addWidget(rLabel);
    rLayout->addWidget(rAddrLine);
    rLayout->addWidget(rAmountLine);
    ui->scrollAreaLayout->addLayout(rLayout);

    m_receiversLabelList.append(rLabel);
    m_receiversAmountLineList.append(rAmountLine);
    m_receiversAddressLineList.append(rAddrLine);
    m_receiversLayoutList.append(rLayout);

    //scroll to bottom
    int s = ui->rScrollArea->verticalScrollBar()->maximum();
    ui->rScrollArea->verticalScrollBar()->setSliderPosition(s);
}

void MultisigTransferWidget::removeReceiverButtonClicked()
{
    int receiverCount = m_receiversLabelList.size();
    if (receiverCount > 0) {
        int toRemoveItem = receiverCount - 1;

        ui->scrollAreaLayout->removeItem(m_receiversLayoutList.at(toRemoveItem));
        m_receiversLayoutList.takeAt(toRemoveItem)->deleteLater();
        m_receiversLabelList.takeAt(toRemoveItem)->deleteLater();
        m_receiversAddressLineList.takeAt(toRemoveItem)->deleteLater();
        m_receiversAmountLineList.takeAt(toRemoveItem)->deleteLater();
    }
}

void MultisigTransferWidget::receiversNextButtonClicked()
{
    //populate map
    m_receiversMap.clear();
    bool valid = true;
    for (int i = 0; i < m_receiversAddressLineList.size(); i++) {
        m_receiversMap.insert(m_receiversAddressLineList.at(i)->text(),
                              m_receiversAmountLineList.at(i)->text());
    }

    //check address format
    int currentReceiver = 0;
    foreach (QLineEdit *l, m_receiversAddressLineList) {
        valid = UtilsIOTA::isValidAddress(l->text());
        currentReceiver++;
        if (!valid) {
            QMessageBox::warning(this, tr("Address Format Error"),
                                 tr("The address of receiver %1 is not valid!")
                                 .arg(currentReceiver));
            break;
        }
    }
    if (!valid) return;

    //exlude own addresses from receivers
    bool ow = false;
    QVariantList ownAddresses = m_walletManager->getPastUsedAddresses();
    ownAddresses.append(m_walletManager->getCurrentAddress());
    foreach (QVariant ows, ownAddresses) {
        if (m_receiversMap.contains(ows.toString())) {
            ow = true;
            QMessageBox::critical(this, tr("Address Error"),
                                  tr("Sending funds to your own addresses "
                                     "is not allowed, this is to avoid losing funds!"));
            break;
        }
    }
    if (ow) return;

    //check amounts
    bool b = true;
    bool error = false;
    quint64 walletBalance = ui->rBalanceLabel->text().toULongLong(&b);
    if (!b) error = true;
    quint64 sendingAmountTot = 0;
    foreach (QLineEdit *a, m_receiversAmountLineList) {
        sendingAmountTot += a->text().toULongLong(&b);
        if (!b) {
            error = true;
            break;
        }
    }
    if ((walletBalance < sendingAmountTot) || (sendingAmountTot == 0)) {
        error = true;
    }
    if (error) {
        QMessageBox::warning(this, tr("Amount Error"),
                             tr("The sending amount is not valid!\n"
                                "Check your wallet balance or amount inputs"));
        return;
    }

    //fill tag
    QString tag = ui->tagLinEdit->text().toUpper();
    int fill9 = 27 - tag.length();
    tag.append(QString("9").repeated(fill9));

    //prepare tx
    m_walletManager->backupMultisigFileAsClean(); //backup clean multisig file
    QVariantList opArgs;
    QStringList addressList = m_receiversMap.keys();
    QStringList amountList = m_receiversMap.values();
    opArgs.append(ui->rBalanceLabel->text());
    opArgs.append(addressList);
    opArgs.append(amountList);
    opArgs.append(tag);
    m_walletManager->setCurrentWalletOp(WalletManager::ColdSign, opArgs);
    WalletManager::WalletError wError;
    m_walletManager->saveWallet(m_currentWalletPath,
                                false,
                                false,
                                wError);
    if (wError.errorType == WalletManager::WalletError::NoError) {
        nextPage();
    } else {
        //restore clean wallet
        m_walletManager->restoreCleanMultisigFileBackup();
        QMessageBox::critical(this,
                              tr("Wallet Save Error"),
                              wError.errorString);
    }
}

void MultisigTransferWidget::onlineSignQuitButtonClicked()
{
    qApp->quit();
}

void MultisigTransferWidget::abortCurrentTransaction()
{
    m_walletManager->restoreCleanMultisigFileBackup();
    m_walletManager->setCurrentWalletOp(WalletManager::WalletOp::NoOp,
                                        QVariantList());
    WalletManager::WalletError error;
    m_walletManager->saveWallet(m_currentWalletPath,
                                false, false, error);
    if (error.errorType != WalletManager::WalletError::NoError) {
        QMessageBox::critical(this,
                              tr("Wallet Save Error"),
                              error.errorString);
    } else {
        emit transferCancelled();
    }
}

void MultisigTransferWidget::offlineSignConfirmButtonClicked()
{
    //check seed
    QString offlineSeed = ui->offlineSeedLineEdit->text().simplified().trimmed().toUpper();
    offlineSeed.replace(" ", "");
    if (!UtilsIOTA::isValidSeed(offlineSeed)) {
        QMessageBox::warning(this,
                             tr("IOTA Seed Invalid"),
                             tr("The entered seed is not a valid format!<br />"
                                "Please check your input and try again"));
        return;
    }

    ui->coldSignResultLabel->hide();
    QStringList args;
    for (int i = 0; i < m_receiverList.size(); i++) {
        args.append(m_amountList.at(i));
        args.append(m_receiverList.at(i));
    }
    args.append("offline"); //offline party
    args.append(m_walletBalance);
    args.append(offlineSeed);
    args.append(m_tag);
    m_walletManager->exportMultisigFile();
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::MultisigTransfer,
                                 args);

    ui->coldSignProgressBar->show();
    ui->coldSignStatusLabel->show();
    ui->coldSignResultLabel->hide();
    nextPage();
}

void MultisigTransferWidget::txFinalQuitButtonClicked()
{
    qApp->quit();
}

void MultisigTransferWidget::txFinalNextButtonClicked()
{
    ui->broadcastResultLabel->hide();
    ui->broadcastProgressBar->show();
    ui->broadcastProgressStatusLabel->show();
    ui->broadcastFinishButton->setEnabled(false);
    nextPage();

    QString onlineSeed = m_walletManager->getOnlineSeed();
    QStringList args;
    for (int i = 0; i < m_receiverList.size(); i++) {
        args.append(m_amountList.at(i));
        args.append(m_receiverList.at(i));
    }
    args.append("online"); //online party
    args.append(m_walletBalance);
    args.append(onlineSeed);
    args.append(m_tag);
    m_walletManager->exportMultisigFile();
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::MultisigTransfer,
                                 args);
}

void MultisigTransferWidget::broadcastFinishButtonClicked()
{
    emit transferCompleted();
}

void MultisigTransferWidget::updateBalance()
{
    ui->rBalanceLabel->setText(tr("checking..."));
    QString a = m_walletManager->getCurrentAddress();
    if (UtilsIOTA::isValidAddress(a)) {
        QStringList args;
        args.append(a);
        m_tangleAPI->startAPIRequest(AbstractTangleAPI::GetBalance,
                                     args);
    }
}

void MultisigTransferWidget::requestFinished(AbstractTangleAPI::RequestType request,
                                   const QString &responseMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
    {
        QString balanceIota = responseMessage.split(":").at(2);
        ui->rBalanceLabel->setText(balanceIota);

    }
        break;
    case AbstractTangleAPI::MultisigTransfer:
        if (responseMessage == "SignOK:offline") {
            WalletManager::WalletError error;
            QVariantList prevArgs;
            m_walletManager->getCurrentWalletOp(prevArgs);
            m_walletManager->setCurrentWalletOp(WalletManager::HotSign,
                                                prevArgs);
            m_walletManager->saveWallet(m_currentWalletPath,
                                        true,
                                        false,
                                        error);
            if (error.errorType != WalletManager::WalletError::NoError) {
                QMessageBox::critical(this,
                                      tr("Wallet Save Error"),
                                      error.errorString);
            } else {
                ui->coldSignProgressBar->hide();
                ui->coldSignStatusLabel->hide();
                ui->coldSignResultLabel->show();
                ui->coldSignResultLabel->setText(tr("Transaction successfully signed!"));
                ui->offlineSigningProgressAbortTxButton->setEnabled(false);
                ui->offlineSigningTxNextButton->setEnabled(true);
            }
        } else if (responseMessage.contains("TransferOK")) {
            //extract new address and tx hash
            QStringList sl = responseMessage.split(":"); //TransferOK:tailTxHash:newAddress
            QString tailTxHash = sl.at(1);
            QString newAddress = sl.at(2);

            //update current address
            QString spendingAddress = m_walletManager->getCurrentAddress();
            m_walletManager->addPastUsedAddress(spendingAddress);
            m_walletManager->setCurrentAddress(newAddress);

            //create and save past txs
            QVariantList prevArgs;
            m_walletManager->getCurrentWalletOp(prevArgs);
            QStringList receiverList = prevArgs.at(1).toStringList();
            QStringList amountList = prevArgs.at(2).toStringList();
            QString tag = prevArgs.at(3).toString();
            for (int i = 0; i < receiverList.size(); i++) {
                UtilsIOTA::Transation tx;
                tx.amount = amountList.at(i);
                tx.dateTime = QDateTime::currentDateTime();
                tx.receivingAddress = receiverList.at(i);
                tx.spendingAddress = spendingAddress;
                tx.tag = tag;
                tx.tailTxHash = tailTxHash;
                m_walletManager->addPastSpendingTx(tx);
            }

            //reset op
            m_walletManager->setCurrentWalletOp(WalletManager::NoOp, QVariantList());

            //save
            WalletManager::WalletError wError;
            m_walletManager->saveWallet(m_currentWalletPath,
                                        true,
                                        true,
                                        wError);
            if (wError.errorType != WalletManager::WalletError::NoError) {
                QMessageBox::critical(this,
                                      tr("Wallet Save Error"),
                                      wError.errorString);
            }

            //show result on label with txhash link to explorer
            ui->broadcastResultLabel->show();
            QString resultMessage = tr("Transaction successfully sent!<br />"
                                       "Transaction hash: <a href='https://thetangle.org/transaction/")
                    .append(tailTxHash).append("'>").append(tailTxHash).append("</a>");
            resultMessage.append(tr("<br /><br /><b>NOTE:</b> Your wallet balance will be "
                                    "temporarily zero until the transaction is confirmed"));
            ui->broadcastResultLabel->setText(resultMessage);
            ui->broadcastFinishButton->setEnabled(true);
            ui->broadcastFinishButton->setFocus();
            ui->broadcastProgressBar->hide();
            ui->broadcastProgressStatusLabel->hide();
        }
        break;
    default:
        break;
    }
}

void MultisigTransferWidget::requestError(AbstractTangleAPI::RequestType request,
                                const QString &errorMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
    {
        ui->rBalanceLabel->setText(tr("error..."));
        QString e = errorMessage;
        if (e.contains("--provider")) {
            e = tr("Failed to check balance! "
                   "Please check your node and connection");
        }
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              e);
        break;
    }
    case AbstractTangleAPI::MultisigTransfer:
    {
        QString message = errorMessage;
        if (errorMessage.contains("Address reuse detected")) {
            message = tr("Address reuse detected!<br />"
                         "<b>Please abort the transaction!</b><br />"
                         "Output: ").append(errorMessage);
        }
        if (ui->stackedWidget->currentIndex() == 7)
            ui->stackedWidget->setCurrentIndex(6);
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              message);
    }
        break;
    default:
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              errorMessage);
        break;
    }
}

void MultisigTransferWidget::clearAllReceivers()
{
    foreach (QLabel *l, m_receiversLabelList) {
        Q_UNUSED(l);
        removeReceiverButtonClicked();
    }

    m_receiversMap.clear();
    m_receiversLabelList.clear();
    m_receiversAmountLineList.clear();
    m_receiversAddressLineList.clear();
    m_receiversLayoutList.clear();
}
