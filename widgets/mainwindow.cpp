#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "createwalletwizard.h"
#include "walletwidget.h"
#include "promotereattachdialog.h"
#include "../utils/definitionholder.h"
#include "../utils/utilsiota.h"
#include "../components/settingsmanager.h"
#include "../components/walletmanager.h"
#include "walletpassphrasedialog.h"
#include "multisigtransferwidget.h"
#include "restorewalletwidget.h"
#include "../components/updatemanager.h"
#include "walletrawdatadialog.h"

#include <QtWidgets/QMenuBar>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtWidgets/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtGui/QClipboard>
#include <QtCore/QMimeData>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_createWalletWidget(0),
    m_walletWidget(0),
    m_multisigTransferWidget(0),
    m_restoreWalletWidget(0),
    m_updateManager(0),
    m_settingsManager(0)
{
    ui->setupUi(this);
    m_menuBar = ui->menuBar;
    ui->mainToolBar->hide();
    ui->openWalletButton->setFocus();
    m_walletManager = &WalletManager::getInstance();
    m_currentWalletPath = "/invalid";
    m_settingsManager = new SettingsManager(this);
    UtilsIOTA::currentNodeUrl = DefinitionHolder::DEFAULT_NODE;

    loadWidgets();
    createMenus();
    createConnections();
    loadSettings();
    checkForUpdatesSlot();

    //install eventfilter for clipboard guard on macOS
    centralWidget()->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    WalletManager::getInstance().destroy();
}

void MainWindow::openWallet(const QString &filePath)
{  
    if (!QFileInfo::exists(filePath)) return;

    WalletPassphraseDialog d(this);
    if (d.exec() == QDialog::Accepted) {
        m_walletManager->unlockWallet(d.getWalletPassphrase());

        WalletManager::WalletError error;
        if (m_walletManager->readWalletFile(filePath, error)) {
            SettingsManager sm(this);
            m_currentWalletPath = filePath;

            switch (m_walletManager->getCurrentWalletOp()) {
            case WalletManager::InitOffline:
                ui->stackedWidget->setCurrentWidget(m_createWalletWidget);
                m_createWalletWidget->setOfflineWalletInitStep(filePath);
                break;
            case WalletManager::RecoverOffline:
                if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OfflineSigner) {
                    m_restoreWalletWidget->prepareColdRecovery(m_currentWalletPath);
                } else {
                    m_restoreWalletWidget->showContinueWithOfflineSigner(m_currentWalletPath);
                }
                ui->stackedWidget->setCurrentWidget(m_restoreWalletWidget);
                break;
            case WalletManager::RecoverOnline:
                if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OnlineSigner) {
                    m_restoreWalletWidget->prepareHotRecovery(m_currentWalletPath);
                } else {
                    m_restoreWalletWidget->showContinueWithOnlineSigner(m_currentWalletPath);
                }
                ui->stackedWidget->setCurrentWidget(m_restoreWalletWidget);
                break;
            case WalletManager::RecoverSign:
                if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OfflineSigner) {
                    m_restoreWalletWidget->prepareRecoveryColdSign(m_currentWalletPath);
                } else {
                    m_restoreWalletWidget->showContinueWithRecoveryColdSign(m_currentWalletPath);
                }
                ui->stackedWidget->setCurrentWidget(m_restoreWalletWidget);
                break;
            case WalletManager::RecoverSend:
                if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OnlineSigner) {
                    m_restoreWalletWidget->prepareRecoveryHotSend(m_currentWalletPath);
                } else {
                    m_restoreWalletWidget->showContinueWithRecoveryHotSend(m_currentWalletPath);
                }
                ui->stackedWidget->setCurrentWidget(m_restoreWalletWidget);
                break;
            case WalletManager::ColdSign:
                if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OfflineSigner) {
                    m_multisigTransferWidget->prepareColdTransferSign(m_currentWalletPath);
                } else {
                    m_multisigTransferWidget->showContinueWithOfflineSigner(m_currentWalletPath);
                }
                ui->stackedWidget->setCurrentWidget(m_multisigTransferWidget);
                break;
            case WalletManager::HotSign:
                if (sm.getDeviceRole() == UtilsIOTA::DeviceRole::OnlineSigner) {
                    m_multisigTransferWidget->prepareHotTransferSign(m_currentWalletPath);
                } else {
                    m_multisigTransferWidget->showContinueWithOnlineSigner(m_currentWalletPath);
                }
                ui->stackedWidget->setCurrentWidget(m_multisigTransferWidget);
                break;
            default:
                if (enforceOnlineRole()) {
                    ui->stackedWidget->setCurrentWidget(m_walletWidget);
                    m_walletWidget->setCurrentWalletPath(filePath);
                }
                break;
            }

            //remember last used directory
            m_settingsManager->saveProperty("lastUsedDir", "mainWindow",
                                            QFileInfo(filePath).dir().absolutePath());
        } else {
            switch (error.errorType) {
            case WalletManager::WalletError::WalletInvalidPassphrase:
                QMessageBox::warning(this, tr("Wallet Error"),
                                     tr("Invalid wallet password!<br />"
                                        "Please try again"));
                openWallet(filePath);
                break;
            case WalletManager::WalletError::WalletFileParsingError:
            {
                QString message = tr("Invalid wallet file: make sure your passphrase is correct!\n");
                error.errorString.append(message);
                QMessageBox::critical(this, tr("Wallet Error"),
                                      error.errorString);
            }
                break;
            default:
                QMessageBox::critical(this, tr("Wallet Error"),
                                      error.errorString);
                break;
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //save window geometry
    saveSettings();

    event->accept();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
    {
        //macOS doesn't allow clipboard monitoring
        //when window is not active
        //so when focus is back check
        clipboardGuardCheck();
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (ui->stackedWidget->currentIndex() == 0) {
        if (event->mimeData()->hasUrls()) {
            QList<QUrl> urls = event->mimeData()->urls();
            //find first file
            foreach (QUrl url, urls) {
                QFileInfo info(url.path());
                QString fileSuffix = info.suffix().toUpper();
                if (fileSuffix == "ICWL") {
                    QString filePath = url.path();
#ifdef Q_OS_WIN
                    filePath.remove(0, 1); //on windows: /C:/file_path is returned from mime
#endif // Q_OS_WIN
                    event->acceptProposedAction();
                    checkDeviceRole();
                    openWallet(filePath);
                    break;
                }
            }
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        if (ui->stackedWidget->currentIndex() == 0)
            event->acceptProposedAction();
    }
}

void MainWindow::aboutQtActionTriggered()
{
    qApp->aboutQt();
}

void MainWindow::aboutActionTriggered()
{
    AboutDialog a(this);
    a.exec();
}

void MainWindow::helpActionTriggered()
{
    QUrl helpUrl(DefinitionHolder::HELP_URL);
    QDesktopServices::openUrl(helpUrl);
}

void MainWindow::settingsActionTriggered()
{
    SettingsDialog d(this);
    d.exec();
}

void MainWindow::promoteActionTriggered()
{
    if (enforceOnlineRole()) {
        PromoteReattachDialog d(this);
        d.setPromoteView();
        d.exec();
    }
}

void MainWindow::reattachActionTriggered()
{
    if (enforceOnlineRole()) {
        PromoteReattachDialog d(this);
        d.setReattachView();
        d.exec();
    }
}

void MainWindow::newWalletButtonClicked()
{
    checkDeviceRole();

    //only online role allowed
    if (enforceOnlineRole()) {
        //setup view
        ui->stackedWidget->setCurrentWidget(m_createWalletWidget);
    }
}

void MainWindow::showWalletJsonActionTriggered()
{
    WalletRawDataDialog d(this);
    d.exec();
}

void MainWindow::openWalletButtonClicked()
{
    checkDeviceRole();

    //restore last used directory
    QString dirPath = m_settingsManager->restoreProperty("lastUsedDir", "mainWindow").toString();
    if (dirPath.isEmpty()) {
        dirPath = QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).at(0);
    }

    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open Wallet File"),
                                                dirPath,
                                                tr("IOTAcooler wallet (*.icwl)"));

    if (!file.isEmpty()) {
        openWallet(file);
    }
}

void MainWindow::newWalletWizardCancelled()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::walletCreationCompleted(bool restoreWallet)
{
    statusBar()->showMessage(tr("Wallet successfully created!"));

    if (!restoreWallet) {
        ui->stackedWidget->setCurrentIndex(0);
    } else {
        m_restoreWalletWidget->prepareHotRecoveryInfo(m_currentWalletPath);
        ui->stackedWidget->setCurrentWidget(m_restoreWalletWidget);
    }
}

void MainWindow::closeWallet()
{
    ui->stackedWidget->setCurrentIndex(0);
    m_walletManager->lockWallet();
    m_currentWalletPath = "/invalid";
}

void MainWindow::showStatusMessage(const QString &message)
{
    ui->statusBar->showMessage(message);
}

void MainWindow::makeNewTransaction()
{
    ui->stackedWidget->setCurrentWidget(m_multisigTransferWidget);
    m_multisigTransferWidget->prepareNewTransfer(m_currentWalletPath);
}

void MainWindow::multisigTransferCancelled()
{
    SettingsManager sm(this);
    if (sm.getDeviceRole() == UtilsIOTA::OnlineSigner) {
        ui->stackedWidget->setCurrentWidget(m_walletWidget);
        m_walletWidget->setCurrentWalletPath(m_currentWalletPath);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::multisigTransferCompleted()
{
    SettingsManager sm(this);
    if (sm.getDeviceRole() == UtilsIOTA::OnlineSigner) {
        ui->stackedWidget->setCurrentWidget(m_walletWidget);
        m_walletWidget->setCurrentWalletPath(m_currentWalletPath);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::restoreWalletCancelled()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::restoreWalletCompleted()
{
    SettingsManager sm(this);
    if (sm.getDeviceRole() == UtilsIOTA::OnlineSigner) {
        ui->stackedWidget->setCurrentWidget(m_walletWidget);
        m_walletWidget->setCurrentWalletPath(m_currentWalletPath);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::checkForUpdatesSlot()
{

    if (m_settingsManager->getDeviceRole() == UtilsIOTA::OfflineSigner)
        return;

    if (DefinitionHolder::APP_STORE || DefinitionHolder::SNAP_PKG ||
            (!m_settingsManager->getCheckUpdates()))
        return;

    if (!m_updateManager) {
        m_updateManager = new UpdateManager(this);
        connect(m_updateManager, SIGNAL(noUpdateSignal()),
                this, SLOT(noUpdateFoundSlot()));
        connect(m_updateManager, SIGNAL(updateErrorSignal()),
                this, SLOT(updateErrorSlot()));
        connect(m_updateManager, SIGNAL(updatesAccepted()),
                this, SLOT(close()));
    }

    //start async update check
    statusBar()->showMessage(tr("Checking for updates..."));
    m_updateManager->checkForUpdates();
}

void MainWindow::noUpdateFoundSlot()
{
    statusBar()->showMessage(tr("Your software version is up to date"));
}

void MainWindow::updateErrorSlot()
{
    statusBar()->showMessage(tr("Error while checking for software updates"));
}

void MainWindow::clipboardGuardCheck()
{
    if (m_settingsManager->isClipboardGuardEnabled() &&
            (m_settingsManager->getDeviceRole() == UtilsIOTA::OnlineSigner)) {
        static QString previousAddress = "";
        static bool hideWarning = false;
        if (hideWarning) return;

        QString clipData = qApp->clipboard()->text().trimmed().toUpper();
        if (UtilsIOTA::isValidAddress(clipData)) {
            QString prevA = previousAddress;

            //set new address before message box to
            //avoid infinite focus event loop on macOS
            previousAddress = clipData;

            //check if new address belongs to iotacooler, in that case skip
            bool isExternalAddress = true;
            if (!m_walletManager->isLocked()) {
                isExternalAddress = m_walletManager->getCurrentAddress() != clipData;
            }
            if ((!prevA.isEmpty()) && isExternalAddress) {
                if (prevA != clipData) {
                    //make addresses more readable
                    QString prevReadable, clipReadable;
                    prevReadable = prevA;
                    prevReadable.insert((prevA.length() == 90 ? 30 : 27), "<br />");
                    prevReadable.insert((prevA.length() == 90 ? 66 : 60), "<br />");
                    clipReadable = clipData;
                    clipReadable.insert((clipReadable.length() == 90 ? 30 : 27), "<br />");
                    clipReadable.insert((clipReadable.length() == 90 ? 66 : 60), "<br />");



                    QMessageBox box(this);
                    box.setIcon(QMessageBox::Warning);
                    box.setWindowTitle(tr("Clipboard Guard"));
                    box.setText(tr("A clipboard data change has been detected!<br />"
                                   "Please check the data to make sure no third-party "
                                   "application like a malware secretly altered "
                                   "your copied IOTA address.<br /><br />"
                                   "<b>Previous:</b><br />%1<br />"
                                   "<b>Current:</b><br />%2")
                                .arg(prevReadable)
                                .arg(clipReadable));
                    QPushButton *disableButton = box.addButton(tr("Don't show this again for this session"),
                                                               QMessageBox::RejectRole);
                    QPushButton *okButton = box.addButton(tr("OK"),
                                                          QMessageBox::AcceptRole);
                    box.setDefaultButton(okButton);
                    box.setWindowModality(Qt::WindowModal);
                    box.exec();
                    if (box.clickedButton() == disableButton) {
                        hideWarning = true;
                    }
                }
            }
        }
    }
}

void MainWindow::loadWidgets()
{
    m_createWalletWidget = new CreateWalletWizard(this);
    ui->stackedWidget->addWidget(m_createWalletWidget);
    m_walletWidget = new WalletWidget(this);
    ui->stackedWidget->addWidget(m_walletWidget);
    m_multisigTransferWidget = new MultisigTransferWidget(this);
    ui->stackedWidget->addWidget(m_multisigTransferWidget);
    m_restoreWalletWidget = new RestoreWalletWidget(this);
    ui->stackedWidget->addWidget(m_restoreWalletWidget);
}

void MainWindow::createMenus()
{
    m_fileMenu = ui->menuBar->addMenu(tr("&File"));
    m_quitAction = new QAction(tr("&Quit"), this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    m_quitAction->setMenuRole(QAction::QuitRole);
    m_fileMenu->addAction(m_quitAction);

    m_toolsMenu = ui->menuBar->addMenu(tr("&Tools"));
    m_promoteAction = new QAction(tr("Promote transaction..."));
    m_promoteAction->setShortcut(tr("CTRL+M"));
    m_toolsMenu->addAction(m_promoteAction);
    m_reattachAction = new QAction(tr("Reattach transaction..."));
    m_reattachAction->setShortcut(tr("CTRL+A"));
    m_toolsMenu->addAction(m_reattachAction);
    m_toolsMenu->addSeparator();
    m_showWalletJsonAction = new QAction(tr("Show raw wallet data..."));
    m_toolsMenu->addAction(m_showWalletJsonAction);
    m_toolsMenu->addSeparator();
    m_settingsAction = new QAction(tr("Settings..."));
    m_settingsAction->setMenuRole(QAction::PreferencesRole);
    m_settingsAction->setShortcut(QKeySequence::Preferences);

    m_toolsMenu->addAction(m_settingsAction);

    m_helpMenu = ui->menuBar->addMenu(tr("&Help"));
    m_aboutAction = new QAction(tr("About %1...").arg(DefinitionHolder::NAME), this);
    m_aboutAction->setMenuRole(QAction::AboutRole);
    m_aboutQtAction = new QAction(tr("About Qt..."), this);
    m_aboutQtAction->setMenuRole(QAction::AboutQtRole);
    m_helpAction = new QAction(tr("Online documentation"));
    m_helpAction->setStatusTip(tr("Visit the online wiki pages..."));
    m_helpAction->setShortcut(QKeySequence::HelpContents);
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_helpAction);
    m_helpMenu->addSeparator();
    m_checkUpdatesAction = new QAction(tr("Check for updates"), this);
    m_checkUpdatesAction->setStatusTip(tr("Check for %1 updates")
                                       .arg(DefinitionHolder::NAME));
    m_checkUpdatesAction->setMenuRole(QAction::ApplicationSpecificRole);
    if (!DefinitionHolder::APP_STORE)
        m_helpMenu->addAction(m_checkUpdatesAction);
}

void MainWindow::createConnections()
{
    connect(m_quitAction, &QAction::triggered,
            this, &MainWindow::close);
    connect(m_aboutQtAction, &QAction::triggered,
            this, &MainWindow::aboutQtActionTriggered);
    connect(m_helpAction, &QAction::triggered,
            this, &MainWindow::helpActionTriggered);
    connect(m_aboutAction, &QAction::triggered,
            this, &MainWindow::aboutActionTriggered);
    connect(m_settingsAction, &QAction::triggered,
            this, &MainWindow::settingsActionTriggered);
    connect(m_promoteAction, &QAction::triggered,
            this, &MainWindow::promoteActionTriggered);
    connect(m_reattachAction, &QAction::triggered,
            this, &MainWindow::reattachActionTriggered);
    connect(ui->newWalletButton, &QPushButton::clicked,
            this, &MainWindow::newWalletButtonClicked);
    connect(ui->openWalletButton, &QPushButton::clicked,
            this, &MainWindow::openWalletButtonClicked);
    connect(m_checkUpdatesAction, SIGNAL(triggered()),
                this, SLOT(checkForUpdatesSlot()));
    connect(m_showWalletJsonAction, &QAction::triggered,
            this, &MainWindow::showWalletJsonActionTriggered);

    //wallet creation wizard
    connect(m_createWalletWidget, &CreateWalletWizard::walletCreationCancelled,
            this, &MainWindow::newWalletWizardCancelled);
    connect(m_createWalletWidget, &CreateWalletWizard::walletCreationCompleted,
            this, &MainWindow::walletCreationCompleted);

    //wallet main view
    connect(m_walletWidget, &WalletWidget::walletClosed,
            this, &MainWindow::closeWallet);
    connect(m_walletWidget, &WalletWidget::showStatusMessage,
            this, &MainWindow::showStatusMessage);
    connect(m_walletWidget, &WalletWidget::makeNewTransactionSignal,
            this, &MainWindow::makeNewTransaction);

    //multisig transfer widget
    connect(m_multisigTransferWidget, &MultisigTransferWidget::transferCancelled,
            this, &MainWindow::multisigTransferCancelled);
    connect(m_multisigTransferWidget, &MultisigTransferWidget::transferCompleted,
            this, &MainWindow::multisigTransferCompleted);

    //restore wallet widget
    connect(m_restoreWalletWidget, &RestoreWalletWidget::restoreWalletCancelled,
            this, &MainWindow::restoreWalletCancelled);
    connect(m_restoreWalletWidget, &RestoreWalletWidget::restoreWalletCompleted,
            this, &MainWindow::restoreWalletCompleted);

    //clipboard guard
    connect(qApp->clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::clipboardGuardCheck);
}

void MainWindow::loadSettings()
{
    SettingsManager sm(this);

    //default node
    UtilsIOTA::currentNodeUrl = sm.getDefaultIOTANodeUrl();

    //restore window geometry
    restoreGeometry(m_settingsManager->restoreGeometry("mainWindow"));
    restoreState(m_settingsManager->restoreState("mainWindow"));
}

void MainWindow::saveSettings()
{
    //save window geometry
    m_settingsManager->saveGeometry("mainWindow", saveGeometry());
    m_settingsManager->saveState("mainWindow", saveState());
}

bool MainWindow::enforceOnlineRole()
{
    SettingsManager sm(this);
    UtilsIOTA::DeviceRole role = sm.getDeviceRole();
    if (role == UtilsIOTA::DeviceRole::OfflineSigner) {
        int r = QMessageBox::warning(this, tr("Online role is required"),
                                     tr("This action requires an active internet connection!"
                                        "<br />Current device role is <b>offline signer</b>."
                                        "<br><b>Do you want to change current device role to online signer?</b>"),
                                     QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes) {
            sm.setDeviceRole(UtilsIOTA::DeviceRole::OnlineSigner);
        } else {
            //not allowed
            return false;
        }
    }
    return true;
}

bool MainWindow::enforceOfflineRole()
{
    SettingsManager sm(this);
    UtilsIOTA::DeviceRole role = sm.getDeviceRole();
    if (role == UtilsIOTA::DeviceRole::OnlineSigner) {
        int r = QMessageBox::warning(this, tr("Offline role is required"),
                                     tr("This action should be performed on an always offline device "
                                        "for best security!"
                                        "<br />Current device role is <b>online signer</b>."
                                        "<br><b>Do you want to change current device role to offline signer?</b>"),
                                     QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes) {
            sm.setDeviceRole(UtilsIOTA::DeviceRole::OfflineSigner);
        } else {
            //not allowed
            return false;
        }
    }
    return true;
}

void MainWindow::checkDeviceRole()
{
    SettingsManager sm(this);
    UtilsIOTA::DeviceRole role = sm.getDeviceRole();
    if (role == UtilsIOTA::DeviceRole::Undefined) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("Device Role"));
        box.setText("Welcome to IOTAcooler, the cold transaction signer."
                    "<br />A cold storage wallet has two components:"
                    "<br />1. An online wallet, to prepare and broadcast transactions"
                    "<br />2. An offline signer, which sings transaction only, "
                    "while keeping the seed (private key) offline"
                    "<br /><b>Which role should this device take?</b> ");
        QPushButton *onlineButton = box.addButton(tr("Online wallet"),
                                                         QMessageBox::NoRole);
        QPushButton *offlineButton = box.addButton(tr("Offline signer"),
                                                    QMessageBox::YesRole);
        box.addButton(QMessageBox::Abort);
        box.setDefaultButton(onlineButton);
        box.setWindowModality(Qt::WindowModal);
        box.exec();

        if (box.clickedButton() == onlineButton) {
            sm.setDeviceRole(UtilsIOTA::DeviceRole::OnlineSigner);
        } else if (box.clickedButton() == offlineButton) {
            sm.setDeviceRole(UtilsIOTA::DeviceRole::OfflineSigner);
        } else {
            qApp->quit();
        }
    }
}
