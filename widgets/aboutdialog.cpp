#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "../utils/definitionholder.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->copyrightLabel->setText(DefinitionHolder::COPYRIGHT);
    ui->versionLabel->setText(DefinitionHolder::VERSION);
    ui->buildLabel->setText(QString::number(DefinitionHolder::SOFTWARE_BUILD));
    ui->donateLabel->setText(tr("<a href='%1'>Support IOTAcooler by donating IOTA</a>")
                             .arg(DefinitionHolder::DONATE_URL));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
