#include "createwalletwizard.h"
#include "ui_createwalletwizard.h"
#include "../components/walletmanager.h"
#include "../components/settingsmanager.h"
#include "../components/smidgentangleapi.h"
#include "../utils/definitionholder.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QRadioButton>

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
    connect(ui->nextWInitProgressButton, &QPushButton::clicked,
            this, &CreateWalletWizard::nextWInitProgressButtonClicked);
    connect(ui->offlineInitNexttoSeedButton, &QPushButton::clicked,
            this, &CreateWalletWizard::offlineInitNexttoSeedButtonClicked);
    connect(ui->seedConfCancelButton, &QPushButton::clicked,
            this, &CreateWalletWizard::infoCancelButtonClicked);
    connect(ui->wBackupInfoNextButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wBackupInfoNextButtonClicked);
    connect(ui->savePDFButton, &QPushButton::clicked,
            this, &CreateWalletWizard::savePDFButtonClicked);
    connect(ui->wBackupNextButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wBackupNextButton);
    connect(ui->wBackupconfirmBackButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wBackupconfirmBackButtonClicked);
    connect(ui->wConfirmButton, &QPushButton::clicked,
            this, &CreateWalletWizard::wConfirmButtonClicked);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &CreateWalletWizard::tangleAPIRequestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &CreateWalletWizard::tangleAPIRequestError);
    connect(ui->manualSeedRadio, &QRadioButton::toggled,
            this, &CreateWalletWizard::seedMethodRadioButtonsChanged);

    //hide optional manual seed entries
    ui->manualSeedGroupBox->hide();

    //hide restore info label
    ui->restoreFundsInfoLabel->hide();
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
        ui->offlineInitNexttoSeedButton->setFocus();

        //check if wallet restoring was selecting as next step after wallet creation
        m_restoreWalletNext = false;
        QVariantList opArgs;
        m_walletManager->getCurrentWalletOp(opArgs);
        if (opArgs.size() > 0) {
            if (opArgs.at(0).toString() == "recover")
                m_restoreWalletNext = true;
        }
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
    if (!fileName.contains(".icwl", Qt::CaseInsensitive))
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
    m_restoreWalletNext = ui->restoreWalletRadio->isChecked();
    m_walletManager->unlockWallet(ui->wpLineEdit->text());
    WalletManager::WalletError error;
    bool ok = m_walletManager->createAndInitWallet(ui->wPathLineEdit->text().trimmed(),
                                                   m_restoreWalletNext,
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
    //check manual seeds
    QString onlineSeed, offlineSeed;
    bool manualSeeds = ui->manualSeedRadio->isChecked();
    if (manualSeeds) {
        onlineSeed = ui->manualOnlineSeedLineEdit->text().simplified()
                .replace(" ", "").trimmed().toUpper();
        offlineSeed = ui->manualOfflineSeedLineEdit->text().simplified()
                .replace(" ", "").trimmed().toUpper();
        if (!((UtilsIOTA::isValidSeed(onlineSeed) && UtilsIOTA::isValidSeed(offlineSeed)))
                || (onlineSeed == offlineSeed)) {
            QMessageBox::warning(this, tr("Invalid Seed Format"),
                                 tr("Your entered seeds are not valid, please check "
                                    "your input and try again!"));
            return;
        }
    }

    m_walletInitStepResults.clear();
    ui->stackedWidget->setCurrentIndex(4);

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

    if (!manualSeeds) {
        QStringList args;
        args.append("offline");
        m_tangleAPI->startAPIRequest(AbstractTangleAPI::CreateSeed,
                                     args);
    } else {
        //add seeds and skip generation
        m_walletInitStepResults.append(offlineSeed);
        m_walletInitStepResults.append(onlineSeed);
        startCreateMultisigFileRequestWithOfflineSeed();
        ui->wInitProgressBar->setValue(ui->wInitProgressBar->value() + 2);
    }
}

void CreateWalletWizard::offlineInitProgressCancelled()
{
    m_tangleAPI->stopCurrentAPIRequest();
    emit walletCreationCancelled();
}

void CreateWalletWizard::nextWInitProgressButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

void CreateWalletWizard::offlineInitNexttoSeedButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(8);
    ui->offlineInitWalletButton->setFocus();
}

void CreateWalletWizard::wBackupInfoNextButtonClicked()
{
    //populate backup data
    QString data;
    data.append(tr("<b>IOTAcooler version:</b> %1<br />")
                .arg(DefinitionHolder::VERSION));
    data.append(tr("<b>Offline signer seed:</b> %1<br />")
                .arg(m_walletInitStepResults.at(0)));
    data.append(tr("<b>Online signer seed:</b> %1<br />")
                .arg(m_walletInitStepResults.at(1)));
    data.append(tr("<b>Wallet encryption password:</b> %1<br />")
                .arg(m_walletManager->getCurrentWalletPassphrase()));
    ui->walletBackupTextArea->setText(data);
    ui->stackedWidget->setCurrentIndex(6);
}

void CreateWalletWizard::seedMethodRadioButtonsChanged()
{
    ui->manualSeedGroupBox->setVisible(ui->manualSeedRadio->isChecked());
}

void CreateWalletWizard::savePDFButtonClicked()
{
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save PDF File"),
                                                    documentsDir + "/" + "iotacooler-backup",
                                                    tr("PDF (*.pdf)"));
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".pdf", Qt::CaseInsensitive))
        fileName.append(".pdf");

    //print to pdf
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    ui->walletBackupTextArea->print(&printer);

    QMessageBox::information(this, tr("PDF Save"),
                             tr("PDF successfully created!"));
}

void CreateWalletWizard::wBackupNextButton()
{
    ui->stackedWidget->setCurrentIndex(7);
    ui->wBackupFilePathLabel->setText(m_currentWalletFilePath);
}

void CreateWalletWizard::wBackupconfirmBackButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}

void CreateWalletWizard::wConfirmButtonClicked()
{
    bool match;
    match = ui->wBackupOfflineSeedLineEdit->text().simplified()
            .replace(" ", "") == m_walletInitStepResults.at(0);
    match = match &&
            ui->wBackupOnlineSeedLineEdit->text().simplified()
            .replace(" ", "") == m_walletInitStepResults.at(1);
    match = match &&
            ui->wBackupPasswordLineEdit->text().simplified()
            .replace(" ", "") == m_walletManager->getCurrentWalletPassphrase();

    if (!match) {
        QMessageBox::warning(this,
                             tr("Incorrect Backup Data"),
                             tr("Your backup data is not correct!\n"
                                "Please try again"));
    } else {
        //set wallet init as complete
        if (m_restoreWalletNext) {
            //if restore funds selected, prepare next step
            QVariantList opArgs;
            opArgs.append(0); //start address index for recovery
            opArgs.append(200); //end index
            m_walletManager->setCurrentWalletOp(WalletManager::WalletOp::RecoverOffline,
                                                opArgs);
        } else {
            m_walletManager->setCurrentWalletOp(WalletManager::WalletOp::NoOp,
                                                QVariantList());
        }

        WalletManager::WalletError error;
        bool w = m_walletManager->saveWallet(m_currentWalletFilePath,
                                             false,
                                             false,
                                             error);
        if (w)
            emit walletCreationCompleted(m_restoreWalletNext);
        else
            walletError(error.errorString);
    }
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
            startCreateMultisigFileRequestWithOfflineSeed();
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
        QString mainAddress = response.split(":").at(2);
        m_walletInitStepResults.append(mainAddress);
        m_walletManager->setCurrentAddress(mainAddress);

        //save online seed to wallet
        m_walletManager->setOnlineSeed(m_walletInitStepResults.at(1));

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
            ui->nextWInitProgressButton->setFocus();
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
    Q_UNUSED(request);

    ui->stackedWidget->setCurrentIndex(4);

    ui->wInitProgressBar->hide();
    ui->wInitProgressWaitLabel->hide();
    ui->wInitProgressStatusLabel->hide();
    ui->wInitProgressResultLabel->show();
    ui->wInitProgressResultLabel->setText(tr("Error: \n") + errorMessage);
}

void CreateWalletWizard::startCreateMultisigFileRequestWithOfflineSeed()
{
    //create multisig file
    ui->wInitProgressBar->setValue(ui->wInitProgressBar->value() + 1);
    ui->wInitProgressStatusLabel->setText(tr("Creating multisig file..."));
    QStringList args;
    args.append("offline"); //pass multisig party
    args.append(m_walletInitStepResults.at(0)); //pass offline seed
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::CreateMultisigWallet,
                                 args);
}
