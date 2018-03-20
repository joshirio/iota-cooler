#include "multisigtransferwidget.h"
#include "ui_multisigtransferwidget.h"

MultisigTransferWidget::MultisigTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultisigTransferWidget)
{
    ui->setupUi(this);

    connect(ui->infoCancelButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::transferCancelled);
    connect(ui->infoNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::nextPage);
}

MultisigTransferWidget::~MultisigTransferWidget()
{
    delete ui;
}

void MultisigTransferWidget::prepareNewTransfer()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MultisigTransferWidget::nextPage()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()
                                       + 1);
}
