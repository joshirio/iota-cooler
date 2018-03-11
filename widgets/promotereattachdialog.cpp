#include "promotereattachdialog.h"
#include "ui_promotereattachdialog.h"
#include "../components/smidgentangleapi.h"

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

    //IOTA API
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &PromoteReattachDialog::tangleAPIRequestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &PromoteReattachDialog::tangleAPIRequestError);
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
    m_tangleAPI->stopCurrentAPIRequest();
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
    ui->promoteResultLabel->clear();
    ui->promoteStatusLabel->show();
    ui->promoteProgressBar->show();
    ui->promoteButton->hide();
    ui->promoteLineEdit->hide();
    ui->tailTxHashLabel->hide();
    ui->okButton->setEnabled(false);

    //promote
    QString tailTxHash = ui->promoteLineEdit->text().trimmed();
    QStringList args;
    args.append(tailTxHash);
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::RequestType::Promote,
                                 args);
}

void PromoteReattachDialog::tangleAPIRequestFinished(AbstractTangleAPI::RequestType request,
                                                     const QString &message)
{
    ui->okButton->setEnabled(true);

    switch (request) {
    case AbstractTangleAPI::RequestType::Promote:
        ui->promoteResultLabel->setText(message);
        ui->promoteResultLabel->show();
        ui->promoteStatusLabel->hide();
        ui->promoteProgressBar->hide();
        ui->promoteButton->show();
        ui->promoteLineEdit->show();
        ui->tailTxHashLabel->show();
        break;
    case AbstractTangleAPI::RequestType::Reattach:
        //ui->reattachResultLabel
        break;
    default:
        break;
    }
}

void PromoteReattachDialog::tangleAPIRequestError(AbstractTangleAPI::RequestType request,
                                                  const QString &message)
{
    tangleAPIRequestFinished(request, message);
}
