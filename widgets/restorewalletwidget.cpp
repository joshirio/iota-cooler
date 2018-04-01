#include "restorewalletwidget.h"
#include "ui_restorewalletwidget.h"
#include "../components/settingsmanager.h"
#include "../components/smidgentangleapi.h"
#include "../components/walletmanager.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>

RestoreWalletWidget::RestoreWalletWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RestoreWalletWidget),
    m_startIndex(0),
    m_endIndex(0),
    m_currentIndex(0)
{
    ui->setupUi(this);

    m_currentWalletPath = "/invalid";
    m_tangleAPI = new SmidgenTangleAPI(this);
    m_walletManager = &WalletManager::getInstance();

    connect(ui->onlineInfoQuitButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::quitButtonClicked);
    connect(ui->restoreInfoCancelButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::restoreWalletCancelled);
    connect(ui->restoreInfoNextButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::restoreInfoNextButtonClicked);
    connect(ui->offlineSeedscancelButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::restoreWalletCancelled);
    connect(ui->offlineSeedsNextButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::offlineSeedsNextButtonClicked);
    connect(ui->cancelGenAddrButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::cancelGenAddrButtonClicked);
    connect(ui->nextGenAddrButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::nextGenAddrButtonClicked);
    connect(ui->quitAddressCheckInfoOffline, &QPushButton::clicked,
            this, &RestoreWalletWidget::quitButtonClicked);
    connect(ui->checkAddrNextButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::checkAddrNextButtonClicked);
    connect(ui->checkAddrCancelButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::checkAddrCancelButtonClicked);
    connect(ui->recoverSignOfflineInfoQuitButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::quitButtonClicked);
    connect(ui->recoverSignSeedsNextButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::recoverSignSeedsNextButtonClicked);
    connect(ui->recoverSignSeedsCancelButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::recoverSignSeedsCancelButtonClicked);
    connect(ui->recoveryOnlineStepInfoQuitButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::quitButtonClicked);
    connect(ui->recoveryColdSignNextButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::recoveryColdSignNextButtonClicked);
    connect(ui->recoveryBroadcastFinishButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::restoreWalletCompleted);

    //tangle api
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &RestoreWalletWidget::requestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &RestoreWalletWidget::requestError);
    connect(m_tangleAPI, &AbstractTangleAPI::addressGenerationProgress,
            this, &RestoreWalletWidget::addressGenerationProgress);
}

RestoreWalletWidget::~RestoreWalletWidget()
{
    delete ui;
}

void RestoreWalletWidget::showContinueWithOfflineSigner(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(1);
}

void RestoreWalletWidget::showContinueWithOnlineSigner(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(4);
}

void RestoreWalletWidget::showContinueWithRecoveryColdSign(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(6);
}

void RestoreWalletWidget::showContinueWithRecoveryHotSend(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(9);
}

void RestoreWalletWidget::prepareColdRecovery(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    m_startIndex = m_endIndex = 0;
    ui->stackedWidget->setCurrentIndex(2);

    //read wallet op args
    QVariantList opArgs;
    m_walletManager->getCurrentWalletOp(opArgs);
    if (opArgs.size() >= 2) {
        m_startIndex = opArgs.at(0).toInt();
        m_endIndex = opArgs.at(1).toInt();
    }
}

void RestoreWalletWidget::prepareHotRecoveryInfo(const QString &walletPath)
{
    ui->stackedWidget->setCurrentIndex(0);
    m_currentWalletPath = walletPath;
    ui->restoreInfoNextButton->setFocus();
}

void RestoreWalletWidget::prepareHotRecovery(const QString &walletPath)
{
    m_currentRecoveryStepResults.clear();
    m_currentWalletPath = walletPath;
    ui->stackedWidget->setCurrentIndex(5);
    ui->checkAddrNextButton->setEnabled(false);
    ui->checkAddrResultLabel->hide();

    //load addresses and indexes
    QVariantList opArgs;
    m_walletManager->getCurrentWalletOp(opArgs);
    if (opArgs.size() > 2) {
        m_startIndex = opArgs.takeFirst().toInt();
        m_endIndex = opArgs.takeFirst().toInt();
        m_currentIndex = m_startIndex;
        foreach (QVariant v, opArgs) {
            m_currentRecoveryStepResults.append(v.toString());
        }
        recoverFundsIsAddrSpentRequest();
    } else {
        ui->checkAddrStatusLabel->clear();
        ui->checkAddrResultLabel->setText(tr("Error: Missing opArgs!"));
        ui->checkAddrResultLabel->show();
    }
}

void RestoreWalletWidget::prepareRecoveryColdSign(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    m_currentRecoveryStepResults.clear();
    ui->stackedWidget->setCurrentIndex(7);
}

void RestoreWalletWidget::prepareRecoveryHotSend(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    m_currentRecoveryStepResults.clear();
    ui->stackedWidget->setCurrentIndex(10);

    ui->recoveryBroadcastFinishButton->setEnabled(false);
    ui->recoveryBroadcastResultLabel->hide();
    ui->recoveryBroadcastProgressBar->show();
    ui->recoveryBroadcastStatusLabel->show();

    QVariantList opArgs;
    m_walletManager->getCurrentWalletOp(opArgs);
    QStringList args;
    args.append(m_walletManager->getCurrentAddress()); //receiving address
    args.append(opArgs.at(0).toString()); //trytes
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::RecoverFundsSend,
                                 args);
}

void RestoreWalletWidget::quitButtonClicked()
{
    qApp->quit();
}

void RestoreWalletWidget::restoreInfoNextButtonClicked()
{
    SettingsManager sm(this);
    if (sm.getDeviceRole() == UtilsIOTA::OnlineSigner)
        nextPage();
    else
        prepareColdRecovery(m_currentWalletPath);
}

void RestoreWalletWidget::offlineSeedsNextButtonClicked()
{
    ui->nextGenAddrButton->setEnabled(false);
    m_currentRecoveryStepResults.clear();
    QString onlineSeed, offlineSeed;
    onlineSeed = ui->onlineSeedLineEdit->text().simplified().trimmed().toUpper();
    onlineSeed.replace(" ", "");
    offlineSeed = ui->offlineSeedLineEdit->text().simplified().trimmed().toUpper();
    offlineSeed.replace(" ", "");
    m_currentRecoveryStepResults.append(onlineSeed);
    m_currentRecoveryStepResults.append(offlineSeed);

    //check that online seed is different than the one from newly created wallet
    if (onlineSeed == m_walletManager->getOnlineSeed()) {
        QMessageBox::warning(this,
                             tr("IOTA Seeds Invalid"),
                             tr("The entered online signer seed is the same "
                                "as the one from the newly created wallet!<br />"
                                "Please enter the actual seeds of your previous wallet "
                                "to recover and try again"));
        return;
    }

    if (UtilsIOTA::isValidSeed(onlineSeed) && UtilsIOTA::isValidSeed(offlineSeed)) {
        nextPage();
        ui->genAddrProgressBar->setRange(m_startIndex, m_endIndex);
        ui->genAddressStatusLabel->setText(tr("Generating %1/%2")
                                           .arg(m_startIndex).arg(m_endIndex));
        //get addresses
        QStringList args;
        args.append(QString::number(m_startIndex));
        args.append(QString::number(m_endIndex));
        args.append(onlineSeed);
        args.append(offlineSeed);
        m_tangleAPI->startAPIRequest(AbstractTangleAPI::GenerateAddresses,
                                     args);
    } else {
        QMessageBox::warning(this,
                             tr("IOTA Seeds Invalid"),
                             tr("The entered seeds are not a valid format!<br />"
                                "Please check your inputs and try again"));
    }
}

void RestoreWalletWidget::cancelGenAddrButtonClicked()
{
    stopRequestAndCancelRecovery();
}

void RestoreWalletWidget::nextGenAddrButtonClicked()
{
    nextPage();
}

void RestoreWalletWidget::checkAddrNextButtonClicked()
{
    bool ok;
    QString indexString, address;
    QString balanceStr = m_currentRecoveryStepResults.takeLast();
    quint64 balance = balanceStr.toULongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this,
                              tr("Reading Balance Error"),
                              tr("Error while converting balance to number!"));
        return;
    }

    WalletManager::WalletOp op;
    QVariantList opArgs;
    if (balance == 0) {
        QMessageBox::information(this,
                                 tr("Zero Balance"),
                                 tr("It appears that your previous "
                                    "IOTAcooler wallet balance is zero. "
                                    "<b>No funds will be transferred!</b><br /><br />"
                                    "If this is wrong, please restart the recovery process "
                                    "again and make sure your seeds are correct and "
                                    "that they were exclusively used by IOTAcooler, "
                                    "otherwise change addresses and balances are not "
                                    "correctly identified."));
        op = WalletManager::NoOp;
    } else {
        op = WalletManager::RecoverSign;
        address = m_currentRecoveryStepResults.takeLast();
        indexString = m_currentRecoveryStepResults.takeLast();
        opArgs.append(indexString);
        opArgs.append(m_walletManager->getCurrentAddress()); //receiving address
        opArgs.append(balanceStr);
    }

    m_walletManager->setCurrentWalletOp(op, opArgs);
    //write wallet
    WalletManager::WalletError wError;
    m_walletManager->saveWallet(m_currentWalletPath,
                                false,
                                false,
                                wError);
    if (wError.errorType != WalletManager::WalletError::NoError) {
        QMessageBox::critical(this,
                              tr("Wallet Save Error"),
                              wError.errorString);
        emit restoreWalletCancelled();
    } else {
        if (op == WalletManager::NoOp)
            emit restoreWalletCompleted();
        else
            nextPage();
    }
}

void RestoreWalletWidget::checkAddrCancelButtonClicked()
{
    stopRequestAndCancelRecovery();
}

void RestoreWalletWidget::recoverSignSeedsCancelButtonClicked()
{
    stopRequestAndCancelRecovery();
}

void RestoreWalletWidget::recoveryColdSignNextButtonClicked()
{
    nextPage();
}

void RestoreWalletWidget::recoverSignSeedsNextButtonClicked()
{
    QString onlineSeed, offlineSeed;
    onlineSeed = ui->onlineSeedLineEdit_2->text().simplified().trimmed().toUpper();
    onlineSeed.replace(" ", "");
    offlineSeed = ui->offlineSeedLineEdit_2->text().simplified().trimmed().toUpper();
    offlineSeed.replace(" ", "");
    m_currentRecoveryStepResults.append(onlineSeed);
    m_currentRecoveryStepResults.append(offlineSeed);

    //check that online seed is different than the one from newly created wallet
    if (onlineSeed == m_walletManager->getOnlineSeed()) {
        QMessageBox::warning(this,
                             tr("IOTA Seeds Invalid"),
                             tr("The entered online signer seed is the same "
                                "as the one from the newly created wallet!<br />"
                                "Please enter the actual seeds of your previous wallet "
                                "to recover and try again"));
        return;
    }

    if (UtilsIOTA::isValidSeed(onlineSeed) && UtilsIOTA::isValidSeed(offlineSeed)) {
        ui->recoveryColdSignNextButton->setEnabled(false);
        ui->recoveryColdSignResultLabel->hide();
        ui->recoveryColdSignProgressBar->show();
        nextPage();

        QVariantList opArgs;
        m_walletManager->getCurrentWalletOp(opArgs);
        QStringList args;
        args.append(opArgs.takeFirst().toString()); //address key index
        args.append(opArgs.takeFirst().toString()); //receiving address
        args.append(opArgs.takeFirst().toString()); //balance
        args.append(onlineSeed);
        args.append(offlineSeed);

        m_tangleAPI->startAPIRequest(AbstractTangleAPI::RecoverFundsSign,
                                     args);
    } else {
        QMessageBox::warning(this,
                             tr("IOTA Seeds Invalid"),
                             tr("The entered seeds are not a valid format!<br />"
                                "Please check your inputs and try again"));
    }
}

void RestoreWalletWidget::requestFinished(AbstractTangleAPI::RequestType request,
                                          const QString &responseMessage)
{
    switch (request) {
    case AbstractTangleAPI::GenerateAddresses:
        if (responseMessage.contains("GenAddr:OK:")) {
            //extract addresses and add to wallet op args
            QStringList addressList = responseMessage.split(":", QString::SkipEmptyParts);
            addressList.removeFirst(); //rm GenAddr:OK:
            addressList.removeFirst();
            QVariantList opArgs;
            opArgs.append(m_startIndex); //add key index range
            opArgs.append(m_endIndex);
            foreach (QString s, addressList) {
                opArgs.append(s);
            }
            m_walletManager->setCurrentWalletOp(WalletManager::RecoverOnline,
                                                opArgs);
            //write wallet
            WalletManager::WalletError wError;
            m_walletManager->saveWallet(m_currentWalletPath,
                                        false,
                                        false,
                                        wError);
            if (wError.errorType != WalletManager::WalletError::NoError) {
                QMessageBox::critical(this,
                                      tr("Wallet Save Error"),
                                      wError.errorString);
            } else {
                ui->genAddressStatusLabel->setText(tr("Finished!"));
                ui->nextGenAddrButton->setEnabled(true);
                ui->nextGenAddrButton->setFocus();
            }
        }
        break;
    case AbstractTangleAPI::IsAddressSpent:
        if (responseMessage.contains("AddressSpent:false")) {
            //clean address found
            m_currentRecoveryStepResults.append(responseMessage.split(":").at(2));
            //check balance
            QStringList args;
            args.append(m_currentRecoveryStepResults.last());
            ui->checkAddrStatusLabel->setText(tr("Checking balance..."));
            ui->checkAddrStatusProgressBar->setRange(0, 0);
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::GetBalance,
                                         args);
        } else if (responseMessage.contains("AddressSpent:true")) {
            //address was already used
            if (m_currentIndex < m_endIndex) {
                m_currentIndex++; //check next address index
                recoverFundsIsAddrSpentRequest();
            } else {
                //current address batch were all used
                //request another 200 address batch
                //then redo this check
                int nextStartIndex = m_endIndex + 1;
                int nextEndIndex = nextStartIndex + 199;
                QVariantList opArgs;
                opArgs.append(nextStartIndex);
                opArgs.append(nextEndIndex);
                m_walletManager->setCurrentWalletOp(WalletManager::RecoverOffline,
                                                    opArgs);
                //write wallet
                WalletManager::WalletError wError;
                m_walletManager->saveWallet(m_currentWalletPath,
                                            false,
                                            false,
                                            wError);
                if (wError.errorType != WalletManager::WalletError::NoError) {
                    QMessageBox::critical(this,
                                          tr("Wallet Save Error"),
                                          wError.errorString);
                } else {
                    QMessageBox::information(this,
                                             tr("Next Addresses Batch"),
                                             tr("The current batch of 200 addresses "
                                                "was insufficient to determine the "
                                                "wallet balance. The wallet has been "
                                                "configured to generate the next batch"
                                                " of 200 addresses. Please open "
                                                "the current wallet file with the "
                                                "<b>offline signing</b> device to proceed!"));
                    qApp->quit();
                }
            }
        }
        break;
    case AbstractTangleAPI::GetBalance:
        if (responseMessage.contains("Balance:")) {
            QString address = responseMessage.split(":").at(1);
            QString balanceString = responseMessage.split(":").at(2);
            QString balanceReadableString =
                    responseMessage.split(":").at(3);
            m_currentRecoveryStepResults.clear();
            m_currentRecoveryStepResults.append(QString::number(m_currentIndex));
            m_currentRecoveryStepResults.append(address);
            m_currentRecoveryStepResults.append(balanceString);
            ui->checkAddrStatusProgressBar->setRange(0, 100);
            ui->checkAddrStatusProgressBar->setValue(100);
            ui->checkAddrNextButton->setEnabled(true);
            ui->checkAddrNextButton->setFocus();
            ui->checkAddrStatusLabel->setText(tr("Finished!"));
            ui->checkAddrResultLabel->setText(tr("Wallet balance: <b>%1</b>")
                                              .arg(balanceReadableString));
            ui->checkAddrResultLabel->show();
        }
        break;
    case AbstractTangleAPI::RecoverFundsSign:
        if (responseMessage.at(0) == "[") {
            //save trytes to op args and set next wallet op to send
            QVariantList opArgs;
            opArgs.append(responseMessage);
            m_walletManager->setCurrentWalletOp(WalletManager::RecoverSend,
                                                opArgs);
            //write wallet
            WalletManager::WalletError wError;
            m_walletManager->saveWallet(m_currentWalletPath,
                                        false,
                                        false,
                                        wError);
            if (wError.errorType != WalletManager::WalletError::NoError) {
                QMessageBox::critical(this,
                                      tr("Wallet Save Error"),
                                      wError.errorString);
            } else {
                ui->recoveryColdSignNextButton->setEnabled(true);
                ui->recoveryColdSignResultLabel->show();
                ui->recoveryColdSignProgressBar->hide();
                ui->recoveryColdSignResultLabel->setText(tr("Recovery transaction successfully signed!"));
            }
        }
        break;
    case AbstractTangleAPI::RecoverFundsSend:
        if (responseMessage.contains("RecoveryOK")) {
            QString txHash = responseMessage.split(":", QString::SkipEmptyParts).at(1);

            QString resultMessage = tr("Transaction successfully sent!<br />"
                                       "Transaction hash: <a href='https://thetangle.org/transaction/")
                    .append(txHash).append("'>").append(txHash).append("</a>");
            resultMessage.append(tr("<br /><br /><b>NOTE:</b> Your wallet balance will be "
                                    "temporarily zero until the transaction is confirmed"));
            ui->recoveryBroadcastResultLabel->setText(resultMessage);

            m_walletManager->setCurrentWalletOp(WalletManager::NoOp,
                                                QVariantList());
            //write wallet
            WalletManager::WalletError wError;
            m_walletManager->saveWallet(m_currentWalletPath,
                                        false,
                                        false,
                                        wError);
            if (wError.errorType != WalletManager::WalletError::NoError) {
                QMessageBox::critical(this,
                                      tr("Wallet Save Error"),
                                      wError.errorString);
                emit restoreWalletCancelled();
            } else {
                ui->recoveryBroadcastFinishButton->setEnabled(true);
                ui->recoveryBroadcastFinishButton->setFocus();
                ui->recoveryBroadcastResultLabel->show();
                ui->recoveryBroadcastProgressBar->hide();
                ui->recoveryBroadcastStatusLabel->hide();
            }
        }
        break;
    default:
        break;
    }
}

void RestoreWalletWidget::requestError(AbstractTangleAPI::RequestType request,
                                       const QString &errorMessage)
{
    switch (request) {
    default:
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              errorMessage);
        break;
    }
}

void RestoreWalletWidget::addressGenerationProgress(int currentIndex, int endIndex)
{
    ui->genAddressStatusLabel->setText(tr("Generating %1/%2")
                                       .arg(currentIndex).arg(endIndex));
    ui->genAddrProgressBar->setValue(currentIndex);
}

void RestoreWalletWidget::nextPage()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void RestoreWalletWidget::stopRequestAndCancelRecovery()
{
    m_tangleAPI->stopCurrentAPIRequest();
    emit restoreWalletCancelled();
}

void RestoreWalletWidget::recoverFundsIsAddrSpentRequest()
{
    ui->checkAddrStatusLabel->setText(tr("Checking %1/%2")
                                      .arg(m_currentIndex).arg(m_endIndex));
    ui->checkAddrStatusProgressBar->setRange(m_startIndex, m_endIndex);
    ui->checkAddrStatusProgressBar->setValue(m_currentIndex);

    QStringList args;
    //check address at current index
    args.append(m_currentRecoveryStepResults.at(m_currentIndex - m_startIndex));
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::IsAddressSpent,
                                 args);
}
