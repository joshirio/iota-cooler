#include "restorewalletwidget.h"
#include "ui_restorewalletwidget.h"
#include "../components/settingsmanager.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>

RestoreWalletWidget::RestoreWalletWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RestoreWalletWidget)
{
    ui->setupUi(this);

    m_currentWalletPath = "/invalid";

    connect(ui->onlineInfoQuitButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::quitButtonClicked);
    connect(ui->restoreInfoCancelButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::restoreWalletCancelled);
    connect(ui->restoreInfoNextButton, &QPushButton::clicked,
            this, &RestoreWalletWidget::restoreInfoNextButtonClicked);
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
    ui->stackedWidget->setCurrentIndex(2);
}

void RestoreWalletWidget::prepareHotRecoveryInfo(const QString &walletPath)
{
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

void RestoreWalletWidget::nextPage()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}
