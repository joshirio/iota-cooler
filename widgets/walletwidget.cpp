#include "walletwidget.h"
#include "ui_walletwidget.h"
#include "../components/walletmanager.h"
#include "../components/smidgentangleapi.h"
#include "../utils/utilsiota.h"
#include "../components/settingsmanager.h"

#include <QtWidgets/QLabel>
#include <QtCore/QTimer>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QHeaderView>

WalletWidget::WalletWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WalletWidget),
    m_tangleRefreshTimer(0)
{
    ui->setupUi(this);
    m_currentWalletFilePath = "/invalid";
    m_walletManager = &WalletManager::getInstance();
    m_settingsManager = new SettingsManager(this);

    connect(ui->closeWalletButton, &QPushButton::clicked,
            this, &WalletWidget::closeWalletButtonClicked);
    connect(ui->copyAddressToolButton, &QToolButton::clicked,
            this, &WalletWidget::copyCurrentAddress);
    connect(ui->copyAddressToolButton_2, &QToolButton::clicked,
            this, &WalletWidget::copyCurrentAddress);
    connect(ui->tangleExplorerButton, &QPushButton::clicked,
            this, &WalletWidget::tangleExplorerButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked,
            this, &WalletWidget::sendButtonClicked);
    connect(ui->addressesButton, &QPushButton::clicked,
            this, &WalletWidget::addressesButtonClicked);
    connect(ui->addressesBackButton, &QPushButton::clicked,
            this, &WalletWidget::addressesBackButtonClicked);
    connect(ui->addressesViewTangleButton, &QPushButton::clicked,
            this, &WalletWidget::addressesViewTangleButtonClicked);

    //pastTxTable
    connect(ui->pastTxTableWidget, &QTableWidget::cellDoubleClicked,
            this, &WalletWidget::tangleExplorerButtonClicked);

    //pastUsedAddressesList
    connect(ui->addressesListWidget, &QListWidget::itemDoubleClicked,
            this, &WalletWidget::addressesViewTangleButtonClicked);

    //tangle api
    m_tangleBalanceCheckAPI = new SmidgenTangleAPI(this);
    connect(m_tangleBalanceCheckAPI, &AbstractTangleAPI::requestFinished,
            this, &WalletWidget::requestFinished);
    connect(m_tangleBalanceCheckAPI, &AbstractTangleAPI::requestError,
            this, &WalletWidget::requestError);
    m_tangleHistoryCheckAPI = new SmidgenTangleAPI(this);
    connect(m_tangleHistoryCheckAPI, &AbstractTangleAPI::requestFinished,
            this, &WalletWidget::requestFinished);
    connect(m_tangleHistoryCheckAPI, &AbstractTangleAPI::requestError,
            this, &WalletWidget::requestError);

    //fix fonts for addresses to use monospace (except on linux)
#ifdef Q_OS_WIN
    QString osMonospace = "Consolas";
#endif
#ifdef Q_OS_OSX
    QString osMonospace = "Menlo";
#endif
#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
    QString s;
    s = ui->addressLabel->styleSheet().replace("monospace", osMonospace);
    ui->addressLabel->setStyleSheet(s);
    s = ui->addressLabel_2->styleSheet().replace("monospace", osMonospace);
    ui->addressLabel_2->setStyleSheet(s);
    s = ui->addressesListWidget->styleSheet().replace("monospace", osMonospace);
    ui->addressesListWidget->setStyleSheet(s);
#endif

}

WalletWidget::~WalletWidget()
{
    //stopBalanceRefresher();
    delete ui;
}

void WalletWidget::setCurrentWalletPath(const QString &walletFilePath)
{
    m_incomingTxList.clear();
    m_outgoingTxList.clear();
    m_currentWalletFilePath = walletFilePath;

    //main wallet address
    QString address = m_walletManager->getCurrentAddress();
    QString addressWrapped = QString(address).insert(30, "<br />")
                                             .insert(66, "<br />");
    ui->addressLabel->setText(QString("<a href='%1'>%2</a>")
                              .arg("https://thetangle.org/address/" + address)
                              .arg(addressWrapped));
    ui->addressLabel_2->setText(addressWrapped);

    //load past transactions
    loadPastTxs();

    //load past addresses
    ui->addressesListWidget->clear();
    QVariantList usedAddresses = m_walletManager->getPastUsedAddresses();
    foreach (QVariant v, usedAddresses) {
        ui->addressesListWidget->addItem(v.toString());
    }

    //check balance
    updateBalance();

    //check history
    updateCurrentAddrTxHistory();

    //setup balance and history periodic check
    startTangleRefreshers();

    //check if address/wallet has been misused (main address is spent)
    checkAddressDirty();
}

void WalletWidget::updateBalance()
{
    if (m_currentWalletFilePath != "/invalid") {
        ui->balanceLabel->setText(tr("checking..."));
        QString a = m_walletManager->getCurrentAddress();
        if (UtilsIOTA::isValidAddress(a)) {
            QStringList args;
            args.append(a);
            m_tangleBalanceCheckAPI->startAPIRequest(AbstractTangleAPI::GetAddrTransfersQuick,
                                                     args);
        }
    }
}

void WalletWidget::updateCurrentAddrTxHistory()
{
    if (m_currentWalletFilePath != "/invalid") {
        //ui->balanceLabel->setText(tr("checking..."));
        QString a = m_walletManager->getCurrentAddress();
        if (UtilsIOTA::isValidAddress(a)) {
            QStringList args;
            args.append(a);
            m_tangleHistoryCheckAPI->startAPIRequest(AbstractTangleAPI::GetBalance,
                                                     args);
        }
    }
}

void WalletWidget::closeWalletButtonClicked()
{
    stopTangleRefreshers();
    emit walletClosed();
}

void WalletWidget::copyCurrentAddress()
{
    QGuiApplication::clipboard()->setText(
                m_walletManager->getCurrentAddress());
    emit showStatusMessage(tr("Address copied to clipboard!"));
}

void WalletWidget::tangleExplorerButtonClicked()
{
    QString h, v;
    if (ui->pastTxTableWidget->selectionModel()->hasSelection()) {
        v = "transaction";
        int i = ui->pastTxTableWidget->currentRow();
        h = ui->pastTxTableWidget->item(i, 2)->text();
    } else {
        v = "address";
        h = m_walletManager->getCurrentAddress();
    }
    QDesktopServices::openUrl(QUrl(QString("https://thetangle.org/%1/%2")
                                   .arg(v).arg(h)));
}

void WalletWidget::sendButtonClicked()
{
    stopTangleRefreshers();
    emit makeNewTransactionSignal();
}

void WalletWidget::addressesButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void WalletWidget::addressesBackButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void WalletWidget::addressesViewTangleButtonClicked()
{
    QString a;
    if (ui->addressesListWidget->selectionModel()->hasSelection()) {
        int i = ui->addressesListWidget->currentRow();
        a = ui->addressesListWidget->item(i)->text();
    } else {
        a = m_walletManager->getCurrentAddress();
    }
    QDesktopServices::openUrl(QUrl(QString("https://thetangle.org/address/%1")
                                   .arg(a)));
}

void WalletWidget::requestFinished(AbstractTangleAPI::RequestType request,
                                   const QString &responseMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
    {
        QString balanceHumanReadable = responseMessage.split(":").at(3);
        ui->balanceLabel->setText(balanceHumanReadable);

    }
        break;
    case AbstractTangleAPI::IsAddressSpent:
    {
        if (responseMessage.contains("AddressSpent:true")) {
            QMessageBox::warning(this, tr("Wallet Inconsistent"),
                                 tr("The wallet state is inconsistent!<br />"
                                    "<b>Do not receive or spend funds, otherwise "
                                    "your risk losing all your funds.</b> "
                                    "The current wallet address has already "
                                    "been spent. Please make sure you are using "
                                    "the latest wallet file and that no other "
                                    "software is using the same multisig seeds."
                                    "<br /><br />"
                                    "If this error persists, try recovering your "
                                    "wallet balance from seeds backup."));
            closeWalletButtonClicked();
        }

        //delete sender, see checkAddressDirty()
        QObject *o = this->sender();
        if (o != nullptr) {
            o->deleteLater();
        }
    }
        break;
    case AbstractTangleAPI::GetAddrTransfersQuick:
    {
        QString json = responseMessage;
        json.remove(0, json.indexOf("[") - 1); //rm garbage at beginning
        m_incomingTxList = UtilsIOTA::parseAddrTransfersQuickJson(json);
        loadPastTxs();
        break;
    }
    default:
        break;
    }
}

void WalletWidget::requestError(AbstractTangleAPI::RequestType request,
                                const QString &errorMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
        ui->balanceLabel->setText(tr("error..."));
        if (errorMessage.contains("--provider")) {
            emit showStatusMessage(tr("Failed to check balance! "
                                      "Please check your node and connection"));
        } else {
            QMessageBox::critical(this,
                                  tr("IOTA API Request Error"),
                                  errorMessage);
        }
        break;
    default:
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              errorMessage);
        break;
    }
}

void WalletWidget::startTangleRefreshers()
{
    if (!m_tangleRefreshTimer) {
        m_tangleRefreshTimer = new QTimer(this);
    }
    connect(m_tangleRefreshTimer, &QTimer::timeout,
            this, &WalletWidget::updateBalance);
    connect(m_tangleRefreshTimer, &QTimer::timeout,
            this, &WalletWidget::updateCurrentAddrTxHistory);
    m_tangleRefreshTimer->start(30000); //every 30sec
}

void WalletWidget::stopTangleRefreshers()
{
    if (m_tangleRefreshTimer) {
        m_tangleRefreshTimer->stop();
        disconnect(m_tangleRefreshTimer, &QTimer::timeout,
                   this, &WalletWidget::updateBalance);
        disconnect(m_tangleRefreshTimer, &QTimer::timeout,
                   this, &WalletWidget::updateCurrentAddrTxHistory);
    }
}

void WalletWidget::loadPastTxs()
{
    ui->pastTxTableWidget->clearContents();
    ui->pastTxTableWidget->setRowCount(0);

    if (m_outgoingTxList.isEmpty())
        m_outgoingTxList = m_walletManager->getPastSpendingTxs();
    foreach (UtilsIOTA::Transation tx, m_outgoingTxList) {
        int rows = ui->pastTxTableWidget->rowCount() + 1;
        int currentRow = rows - 1;
        ui->pastTxTableWidget->setRowCount(rows);
        QTableWidgetItem *date = new QTableWidgetItem(tx.dateTime
                                                      .toString(Qt::DefaultLocaleShortDate));
        QTableWidgetItem *amount = new QTableWidgetItem(tx.amount);
        QTableWidgetItem *txHash = new QTableWidgetItem(tx.tailTxHash);
        QTableWidgetItem *from = new QTableWidgetItem(tx.spendingAddress);
        QTableWidgetItem *to = new QTableWidgetItem(tx.receivingAddress);
        QTableWidgetItem *tag = new QTableWidgetItem(UtilsIOTA::getEasyReadableTag(tx.tag));
        ui->pastTxTableWidget->setItem(currentRow, 0, date);
        ui->pastTxTableWidget->setItem(currentRow, 1, amount);
        ui->pastTxTableWidget->setItem(currentRow, 2, txHash);
        ui->pastTxTableWidget->setItem(currentRow, 3, from);
        ui->pastTxTableWidget->setItem(currentRow, 4, to);
        ui->pastTxTableWidget->setItem(currentRow, 5, tag);
    }

    //TODO append to incoming tx from wallet manager saved ones
    foreach (UtilsIOTA::Transation tx, m_incomingTxList) {
        int rows = ui->pastTxTableWidget->rowCount() + 1;
        int currentRow = rows - 1;
        ui->pastTxTableWidget->setRowCount(rows);
        QTableWidgetItem *date = new QTableWidgetItem(tx.dateTime
                                                      .toString(Qt::DefaultLocaleShortDate));
        QTableWidgetItem *amount = new QTableWidgetItem(tx.amount);
        QTableWidgetItem *txHash = new QTableWidgetItem(tx.tailTxHash); //not real tail when quick method
        QTableWidgetItem *from = new QTableWidgetItem(tx.spendingAddress);
        QTableWidgetItem *to = new QTableWidgetItem(tx.receivingAddress);
        QTableWidgetItem *tag = new QTableWidgetItem(UtilsIOTA::getEasyReadableTag(tx.tag));
        ui->pastTxTableWidget->setItem(currentRow, 0, date);
        ui->pastTxTableWidget->setItem(currentRow, 1, amount);
        ui->pastTxTableWidget->setItem(currentRow, 2, txHash);
        ui->pastTxTableWidget->setItem(currentRow, 3, from);
        ui->pastTxTableWidget->setItem(currentRow, 4, to);
        ui->pastTxTableWidget->setItem(currentRow, 5, tag);
    }

    ui->pastTxTableWidget->horizontalHeader()->setSectionResizeMode(0,
                                                                    QHeaderView::ResizeToContents);
    ui->pastTxTableWidget->horizontalHeader()->setSectionResizeMode(1,
                                                                    QHeaderView::ResizeToContents);
}

void WalletWidget::checkAddressDirty()
{
    AbstractTangleAPI *a = new SmidgenTangleAPI(this);
    connect(a, &AbstractTangleAPI::requestFinished,
            this, &WalletWidget::requestFinished);
    connect(a, &AbstractTangleAPI::requestError,
            this, &WalletWidget::requestError);

    QStringList args;
    args.append(m_walletManager->getCurrentAddress());
    a->startAPIRequest(AbstractTangleAPI::IsAddressSpent, args);
}
