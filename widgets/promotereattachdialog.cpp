#include "promotereattachdialog.h"
#include "ui_promotereattachdialog.h"

#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDialogButtonBox>
#include <QtCore/QRegularExpression>

PromoteReattachDialog::PromoteReattachDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PromoteReattachDialog)
{
    ui->setupUi(this);
    ui->okButton->setAutoDefault(false);
    ui->okButton->setDefault(false);
    ui->cancelButton->setAutoDefault(false);
    ui->cancelButton->setDefault(false);

    connect(ui->okButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::okButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::cancelButtonClicked);
    connect(ui->promoteButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::promoteButtonClicked);
    connect(ui->promoteLineEdit, &QLineEdit::textChanged,
            this, &PromoteReattachDialog::promoteLineEditChanged);
}

PromoteReattachDialog::~PromoteReattachDialog()
{
    delete ui;
}

void PromoteReattachDialog::setPromoteView()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->promoteResultLabel->clear();
    ui->promoteResultLabel->hide();
    ui->promoteProgressBar->hide();
    ui->promoteStatusLabel->hide();
    ui->promoteButton->setEnabled(false);
}

void PromoteReattachDialog::setReattachView()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void PromoteReattachDialog::okButtonClicked()
{
    this->accept();
}

void PromoteReattachDialog::cancelButtonClicked()
{
    //TODO: should stop current operation (promote or reattach)
    this->reject();
}

void PromoteReattachDialog::promoteLineEditChanged()
{
    QString txHash = ui->promoteLineEdit->text().trimmed().toUpper();
    //check if tx is valid
    QRegularExpression reg("^[A-Z9]{81}$");
    bool valid = reg.match(txHash).hasMatch();
    ui->promoteButton->setEnabled(valid);
}

void PromoteReattachDialog::promoteButtonClicked()
{
    ui->promoteStatusLabel->show();
    ui->promoteProgressBar->show();
    ui->promoteButton->hide();
    ui->promoteLineEdit->hide();
    ui->tailTxHashLabel->hide();
    ui->okButton->setEnabled(false);
}
