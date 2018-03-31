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
    //TODO prepare recovery op and save wallet state etc...
}

void RestoreWalletWidget::checkAddrCancelButtonClicked()
{
    stopRequestAndCancelRecovery();
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
                //the redo this check
                //TODO
            }
        }
        break;
    case AbstractTangleAPI::GetBalance:
        if (responseMessage.contains("Balance:")) {
            QString balanceString = responseMessage.split(":").at(2);
            QString balanceReadableString =
                    responseMessage.split(":").at(3);
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
