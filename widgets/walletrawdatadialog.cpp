#include "walletrawdatadialog.h"
#include "ui_walletrawdatadialog.h"
#include "../components/walletmanager.h"

WalletRawDataDialog::WalletRawDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WalletRawDataDialog)
{
    ui->setupUi(this);

    //load wallet raw json data
    ui->label->setText(WalletManager::getInstance()
                       .getRawJsonData());
}

WalletRawDataDialog::~WalletRawDataDialog()
{
    delete ui;
}
