#include "walletwidget.h"
#include "ui_walletwidget.h"

WalletWidget::WalletWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WalletWidget)
{
    ui->setupUi(this);
    m_currentWalletFilePath = "/invalid";
}

WalletWidget::~WalletWidget()
{
    delete ui;
}

void WalletWidget::setCurrentWalletPath(const QString &walletFilePath)
{
    m_currentWalletFilePath = walletFilePath;
}
