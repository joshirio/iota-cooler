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
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>

CreateWalletWizard::CreateWalletWizard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateWalletWizard)
{
    ui->setupUi(this);

    m_currentWalletFilePath = "/invalid";
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
    connect(ui->cancelOfflineWInitProgressButton, &QPushButton::clicked,
            this, &CreateWalletWizard::offlineInitProgressCancelled);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &CreateWalletWizard::tangleAPIRequestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &CreateWalletWizard::tangleAPIRequestError);
}

CreateWalletWizard::~CreateWalletWizard()
{
    delete ui;
}

void CreateWalletWizard::setOfflineWalletInitStep(const QString &walletFilePath)
{
    SettingsManager sm(this);
    if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OfflineSigner) {
        m_currentWalletFilePath = walletFilePath;
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

    //status update
    //5 steps:
    //seed gen offline
    //seed gen online
    //multisig create
    //multisig add
    //multisig finalize
    ui->wInitProgressBar->setRange(0, 5);
    ui->wInitProgressBar->setValue(1);
    ui->wInitProgressResultLabel->hide();
    ui->wInitProgressStatusLabel->setText(tr("Generating offline signer seed..."));

    QStringList args;
    args.append("offline");
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::CreateSeed,
                                 args);
}

void CreateWalletWizard::offlineInitProgressCancelled()
{
    m_tangleAPI->stopCurrentAPIRequest();
    emit walletCreationCancelled();
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
    switch (request) {
    case AbstractTangleAPI::CreateSeed:
        if (response.contains("offline")) {
            //extract offline seed
            m_walletInitStepResults.append(response.split(":").at(2));

            //generate online seed
            ui->wInitProgressBar->setValue(ui->wInitProgressBar->value() + 1);
            ui->wInitProgressStatusLabel->setText(tr("Generating online signer seed..."));
            QStringList args;
            args.append("online");
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::CreateSeed,
                                         args);

        } else if (response.contains("online")) {
            //extract online seed
            m_walletInitStepResults.append(response.split(":").at(2));

            //create multisig file
            ui->wInitProgressBar->setValue(ui->wInitProgressBar->value() + 1);
            ui->wInitProgressStatusLabel->setText(tr("Creating multisig file..."));
            QStringList args;
            args.append("offline"); //pass multisig party
            args.append(m_walletInitStepResults.at(0)); //pass offline seed
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::CreateMultisigWallet,
                                         args);
        }
        break;
    case AbstractTangleAPI::CreateMultisigWallet:
        if (response.contains("OK")) {
            //add online signing party to multisig
            ui->wInitProgressBar->setValue(ui->wInitProgressBar->value() + 1);
            ui->wInitProgressStatusLabel->setText(tr("Adding multisig parties..."));
            QStringList args;
            args.append("online"); //pass multisig party
            args.append(m_walletInitStepResults.at(1)); //pass online seed
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::AddMultisigParty,
                                         args);
        }
        break;
    case AbstractTangleAPI::AddMultisigParty:
        if (response.contains("OK")) {
            //finalize multisig
            ui->wInitProgressBar->setValue(ui->wInitProgressBar->value() + 1);
            ui->wInitProgressStatusLabel->setText(tr("Finalizing multisig file..."));
            QStringList args;
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::FinalizeMultsigWallet,
                                         args);
        }
        break;
    case AbstractTangleAPI::FinalizeMultsigWallet:
    {
        //extract main wallet address
        m_walletInitStepResults.append(response.split(":").at(2));

        //save wallet
        WalletManager::WalletError error;
        bool w = m_walletManager->saveWallet(m_currentWalletFilePath,
                                             true,
                                             true,
                                             error);
        if (w) {
            //finished
            ui->cancelOfflineWInitProgressButton->setEnabled(false);
            ui->wInitProgressResultLabel->show();
            ui->wInitProgressResultLabel->setText(tr("Wallet successfully initialized!"));
            ui->wInitProgressWaitLabel->setText(tr("Finished!"));
            ui->wInitProgressStatusLabel->hide();
            ui->nextWInitProgressButton->setEnabled(true);

            //TODO setup view for wallet seed, pass and general info about backup

        } else {
            tangleAPIRequestError(request, error.errorString);
        }
    }
        break;
    default:
        break;
    }
}

void CreateWalletWizard::tangleAPIRequestError(AbstractTangleAPI::RequestType request,
                                               const QString &errorMessage)
{
    ui->stackedWidget->setCurrentIndex(4);

    ui->wInitProgressBar->hide();
    ui->wInitProgressWaitLabel->hide();
    ui->wInitProgressStatusLabel->hide();
    ui->wInitProgressResultLabel->show();
    ui->wInitProgressResultLabel->setText(tr("Error: \n") + errorMessage);
}
