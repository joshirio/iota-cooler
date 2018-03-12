#include "createwalletwizard.h"
#include "ui_createwalletwizard.h"

CreateWalletWizard::CreateWalletWizard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateWalletWizard)
{
    ui->setupUi(this);
}

CreateWalletWizard::~CreateWalletWizard()
{
    delete ui;
}
