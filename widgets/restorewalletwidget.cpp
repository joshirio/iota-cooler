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
    m_currentIndex(0),
    m_maxIndex(0)
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

    //tangle api
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &MultisigTransferWidget::requestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &MultisigTransferWidget::requestError);
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
    //TODO
}

void RestoreWalletWidget::prepareColdRecovery(const QString &walletPath)
{
    m_currentWalletPath = walletPath;
    m_currentIndex = m_maxIndex = 0;
    ui->stackedWidget->setCurrentIndex(2);

    //read wallet op args
    QVariantList opArgs;
    m_walletManager->getCurrentWalletOp(opArgs);
    if (opArgs.size() >= 2) {
        m_currentIndex = opArgs.at(0).toInt();
        m_maxIndex = opArgs.at(1).toInt();
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
    m_currentWalletPath = walletPath;
    //TODO
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
    m_currentRecoveryStepResults.clear();
    QString onlineSeed, offlineSeed;
    onlineSeed = ui->onlineSeedLineEdit->text().simplified().trimmed().toUpper();
    onlineSeed.replace(" ", "");
    offlineSeed = ui->onlineSeedLineEdit->text().simplified().trimmed().toUpper();
    offlineSeed.replace(" ", "");

    if (UtilsIOTA::isValidSeed(onlineSeed) && UtilsIOTA::isValidSeed(offlineSeed)) {
        nextPage();
        ui->genAddressStatusLabel->setText(tr("Generating %1/%2")
                                           .arg(m_currentIndex).arg(m_maxIndex));
        //TODO start api request
    } else {
        QMessageBox::warning(this,
                             tr("IOTA Seeds Invalid"),
                             tr("The entered seeds are not a valid format!<br />"
                                "Please check your inputs and try again"));
    }
}

void RestoreWalletWidget::cancelGenAddrButtonClicked()
{
    m_tangleAPI->stopCurrentAPIRequest();
    emit restoreWalletCancelled();
}

void RestoreWalletWidget::requestFinished(AbstractTangleAPI::RequestType request,
                                          const QString &responseMessage)
{
    switch (request) {
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

void RestoreWalletWidget::nextPage()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}
