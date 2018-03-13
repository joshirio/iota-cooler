#include "createwalletwizard.h"
#include "ui_createwalletwizard.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>

CreateWalletWizard::CreateWalletWizard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateWalletWizard)
{
    ui->setupUi(this);

    connect(ui->infoNextButtonClicked, &QPushButton::clicked,
            this, &CreateWalletWizard::infoNextButtonClicked);
    connect(ui->infoCancelButtonClicked, &QPushButton::clicked,
            this, &CreateWalletWizard::infoCancelButtonClicked);
}

CreateWalletWizard::~CreateWalletWizard()
{
    delete ui;
}

void CreateWalletWizard::infoNextButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void CreateWalletWizard::infoCancelButtonClicked()
{
    emit walletCreationCancelled();
}
