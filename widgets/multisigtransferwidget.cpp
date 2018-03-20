#include "multisigtransferwidget.h"
#include "ui_multisigtransferwidget.h"
#include "../components/smidgentangleapi.h"
#include "../utils/utilsiota.h"
#include "../components/walletmanager.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QRegExpValidator>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>

MultisigTransferWidget::MultisigTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultisigTransferWidget)
{
    ui->setupUi(this);
    m_walletManager = &WalletManager::getInstance();

    connect(ui->infoCancelButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::transferCancelled);
    connect(ui->infoNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::nextPage);
    connect(ui->rAddButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::addReceiverButtonClicked);
    connect(ui->rRemoveButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::removeReceiverButtonClicked);
    connect(ui->rCancelButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::transferCancelled);
    connect(ui->rNextButton, &QPushButton::clicked,
            this, &MultisigTransferWidget::receiversNextButtonClicked);

    //tangle api
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &MultisigTransferWidget::requestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &MultisigTransferWidget::requestError);
}

MultisigTransferWidget::~MultisigTransferWidget()
{
    delete ui;
}

void MultisigTransferWidget::prepareNewTransfer()
{
    ui->stackedWidget->setCurrentIndex(0);
    clearAllReceivers();
    addReceiverButtonClicked();
    updateBalance();
    ui->infoNextButton->setFocus();
}

void MultisigTransferWidget::nextPage()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()
                                       + 1);
}

void MultisigTransferWidget::addReceiverButtonClicked()
{
    int receiverCount = m_receiversLabelList.size();
    int currentReceiver = receiverCount + 1;

    QRegExp addrRe("^[A-Z9]{90}$");
    QRegExpValidator *addrValidator = new QRegExpValidator(addrRe, this);
    QRegExp amountRe("^[0-9]*$");
    QRegExpValidator *amountValidator = new QRegExpValidator(amountRe, this);

    QLabel *rLabel = new QLabel(tr("Receiver ") +
                                QString::number(currentReceiver) + ":",
                                this);
    QLineEdit *rAddrLine = new QLineEdit(this);
    rAddrLine->setPlaceholderText(tr("Receiver %1 address").arg(currentReceiver));
    rAddrLine->setClearButtonEnabled(true);
    rAddrLine->setValidator(addrValidator);
    QLineEdit *rAmountLine = new QLineEdit(this);
    rAmountLine->setPlaceholderText(tr("Receiver %1 amount (iota)").arg(currentReceiver));
    rAmountLine->setClearButtonEnabled(true);
    rAmountLine->setValidator(amountValidator);
    QHBoxLayout *rLayout = new QHBoxLayout(); //becomes child of scrollAreaLayout (no leak)
    rLayout->addWidget(rLabel);
    rLayout->addWidget(rAddrLine);
    rLayout->addWidget(rAmountLine);
    ui->scrollAreaLayout->addLayout(rLayout);

    m_receiversLabelList.append(rLabel);
    m_receiversAmountLineList.append(rAmountLine);
    m_receiversAddressLineList.append(rAddrLine);
    m_receiversLayoutList.append(rLayout);

    //scroll to bottom
    int s = ui->rScrollArea->verticalScrollBar()->maximum();
    ui->rScrollArea->verticalScrollBar()->setSliderPosition(s);
}

void MultisigTransferWidget::removeReceiverButtonClicked()
{
    int receiverCount = m_receiversLabelList.size();
    if (receiverCount > 0) {
        int toRemoveItem = receiverCount - 1;

        ui->scrollAreaLayout->removeItem(m_receiversLayoutList.at(toRemoveItem));
        m_receiversLayoutList.takeAt(toRemoveItem)->deleteLater();
        m_receiversLabelList.takeAt(toRemoveItem)->deleteLater();
        m_receiversAddressLineList.takeAt(toRemoveItem)->deleteLater();
        m_receiversAmountLineList.takeAt(toRemoveItem)->deleteLater();
    }
}

void MultisigTransferWidget::receiversNextButtonClicked()
{
    //populate map
    bool valid = true;
    for (int i = 0; i < m_receiversAddressLineList.size(); i++) {
        m_receiversMap.insert(m_receiversAddressLineList.at(i)->text(),
                              m_receiversAmountLineList.at(i)->text());
    }

    //check address format
    int currentReceiver = 0;
    foreach (QLineEdit *l, m_receiversAddressLineList) {
        valid = UtilsIOTA::isValidAddress(l->text());
        currentReceiver++;
        if (!valid) {
            QMessageBox::warning(this, tr("Address Format Error"),
                                 tr("The address of receiver %1 is not valid!")
                                 .arg(currentReceiver));
            break;
        }
    }
    if (!valid) return;

    //check amounts
    bool b = true;
    bool error = false;
    quint64 walletBalance = ui->rBalanceLabel->text().toULongLong(&error);
    quint64 sendingAmountTot = 0;
    foreach (QLineEdit *a, m_receiversAmountLineList) {
        sendingAmountTot += a->text().toULongLong(&b);
        if (!b) {
            error = true;
            break;
        }
    }
    if (walletBalance < sendingAmountTot) {
        error = true;
    }
    if (error) {
        QMessageBox::warning(this, tr("Amount Error"),
                             tr("The sending amount is not valid!\n"
                                "Check if your wallet balance or input format"));
        return;
    }

    //prepare tx
    //TODO

    nextPage();
}

void MultisigTransferWidget::updateBalance()
{
    ui->rBalanceButton->setText(tr("checking..."));
    QString a = m_walletManager->getCurrentAddress();
    if (UtilsIOTA::isValidAddress(a)) {
        QStringList args;
        args.append(a);
        m_tangleAPI->startAPIRequest(AbstractTangleAPI::GetBalance,
                                     args);
    }
}

void MultisigTransferWidget::requestFinished(AbstractTangleAPI::RequestType request,
                                   const QString &responseMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
    {
        QString balanceIota = responseMessage.split(":").at(2);
        ui->rBalanceButton->setText(balanceIota);

    }
        break;
    default:
        break;
    }
}

void MultisigTransferWidget::requestError(AbstractTangleAPI::RequestType request,
                                const QString &errorMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
    {
        ui->rBalanceButton->setText(tr("error..."));
        QString e = errorMessage;
        if (e.contains("--provider")) {
            e = tr("Failed to check balance! "
                   "Please check your node and connection");
        }
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              e);
        break;
    }
    default:
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              errorMessage);
        break;
    }
}

void MultisigTransferWidget::clearAllReceivers()
{
    foreach (QLabel *l, m_receiversLabelList) {
        Q_UNUSED(l);
        removeReceiverButtonClicked();
    }

    m_receiversMap.clear();
    m_receiversLabelList.clear();
    m_receiversAmountLineList.clear();
    m_receiversAddressLineList.clear();
    m_receiversLayoutList.clear();
}
