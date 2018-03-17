#include "createwalletwizard.h"
#include "ui_createwalletwizard.h"
#include "../components/walletmanager.h"
#include "../components/settingsmanager.h"
#include "../components/smidgentangleapi.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

CreateWalletWizard::CreateWalletWizard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateWalletWizard)
{
    ui->setupUi(this);

    m_walletManager = &WalletManager::getInstance();
    m_tangleAPI = new SmidgenTangleAPI(this);

    connect(ui->infoNextButtonClicked, &QPushButton::clicked,
            this, &CreateWalletWizard::infoNextButtonClicked);
    connect(ui->infoCancelButtonClicked, &QPushButton::clicked,
            this, &CreateWalletWizard::infoCancelButtonClicked);
    connect(ui->wConfCancelButton, &QPushButton::clicked,
            this, &CreateWalletWizard::infoCancelButtonClicked);
    connect(ui->wConfBrowseButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wConfBrowseButtonClicked);
    connect(ui->wPathLineEdit, &QLineEdit::textChanged,
            this, &CreateWalletWizard::wConfUpdateNextButtonState);
    connect(ui->wpLineEdit, &QLineEdit::textChanged,
            this, &CreateWalletWizard::wConfUpdateNextButtonState);
    connect(ui->wp2LineEdit, &QLineEdit::textChanged,
            this, &CreateWalletWizard::wConfUpdateNextButtonState);
    connect(ui->wConfNextButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wConfNextButtonClicked);
    connect(ui->wInitOnlineQuitButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wInitOnlineQuitButtonClicked);
    connect(ui->cancelOfflineWalletInit, &QPushButton::clicked,
            this, &CreateWalletWizard::infoCancelButtonClicked);
    connect(ui->offlineInitWalletButton, &QPushButton::clicked,
            this, &CreateWalletWizard::offlineInitWalletButtonClicked);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &CreateWalletWizard::tangleAPIRequestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &CreateWalletWizard::tangleAPIRequestError);
}

CreateWalletWizard::~CreateWalletWizard()
{
    delete ui;
}

void CreateWalletWizard::setOfflineWalletInitStep()
{
    SettingsManager sm(this);
    if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OfflineSigner) {
        ui->stackedWidget->setCurrentIndex(3);
        ui->offlineInitWalletButton->setFocus();
    } else {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void CreateWalletWizard::infoNextButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void CreateWalletWizard::infoCancelButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    emit walletCreationCancelled();
}

void CreateWalletWizard::wConfBrowseButtonClicked()
{
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Wallet File"),
                                                    documentsDir + "/" + "coldwallet",
                                                    tr("IOTAcooler Wallet(*.icwl)"));
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".icwl"))
        fileName.append(".icwl");
    ui->wPathLineEdit->setText(fileName);
}

void CreateWalletWizard::wConfUpdateNextButtonState()
{
    //check pass equal
    bool passValid = ui->wpLineEdit->text() == ui->wp2LineEdit->text();
    passValid = passValid && (!ui->wpLineEdit->text().trimmed().isEmpty());

    ui->wConfNextButton->setEnabled(passValid &&
                                    (!ui->wPathLineEdit->text().trimmed().isEmpty()));
}

void CreateWalletWizard::wConfNextButtonClicked()
{
    m_walletManager->unlockWallet(ui->wpLineEdit->text());
    WalletManager::WalletError error;
    bool ok = m_walletManager->createAndInitWallet(ui->wPathLineEdit->text().trimmed(),
                                                   error);
    m_walletManager->lockWallet();

    if (ok) {
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        switch (error.errorType) {
        case WalletManager::WalletError::WalletFileParsingError:
            walletParseError(error.errorString);
            break;
        case WalletManager::WalletError::WalletInvalidPassphrase:
            walletPassError();
            break;
        default:
            this->walletError(error.errorString);
            break;
        }
    }
}

void CreateWalletWizard::wInitOnlineQuitButtonClicked()
{
    qApp->quit();
}

void CreateWalletWizard::offlineInitWalletButtonClicked()
{
    m_walletInitStepResults.clear();
    ui->stackedWidget->setCurrentIndex(4);

    QStringList args;
    args.append("online");
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::CreateSeed,
                                 args);
}

void CreateWalletWizard::walletError(const QString &message)
{
    QMessageBox::critical(this, tr("Wallet Error"), message);
    emit walletCreationCancelled();
}

void CreateWalletWizard::walletParseError(const QString &message)
{
    QString err = tr("Invalid wallet file: make sure your passphrase is correct!\n");
    err.append(message);
    walletError(err);
}

void CreateWalletWizard::walletPassError()
{
    walletParseError("");
}

void CreateWalletWizard::tangleAPIRequestFinished(AbstractTangleAPI::RequestType request,
                                                  const QString &response)
{
    //TODO

    switch (request) {
    case AbstractTangleAPI::CreateSeed:
        if (response.contains("online")) {
            //extract online seed
            m_walletInitStepResults.append(response.split(":").at(2));
            //TODO update view status and generate offline seed
        } else if (response.contains("offline")) {
            //extract offline seed
            m_walletInitStepResults.append(response.split(":").at(2));
            //TODO
        }
        break;
    default:
        break;
    }
}

void CreateWalletWizard::tangleAPIRequestError(AbstractTangleAPI::RequestType request,
                                               const QString &errorMessage)
{
    //TODO
}
