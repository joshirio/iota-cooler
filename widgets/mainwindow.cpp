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

#include <QtWidgets/QMenuBar>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtWidgets/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_createWalletWidget(0),
    m_walletWidget(0)
{
    ui->setupUi(this);
    m_menuBar = ui->menuBar;
    ui->mainToolBar->hide();
    ui->openWalletButton->setFocus();
    m_walletManager = &WalletManager::getInstance();

    loadWidgets();
    createMenus();
    createConnections();
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
    WalletManager::getInstance().destroy();
}

void MainWindow::openWallet(const QString &filePath)
{  
    WalletPassphraseDialog d(this);
    if (d.exec() == QDialog::Accepted) {
        m_walletManager->unlockWallet(d.getWalletPassphrase());

        WalletManager::WalletError error;
        if (m_walletManager->readWalletFile(filePath, error)) {
            switch (m_walletManager->getCurrentWalletOp()) {
            case WalletManager::InitOffline:
                ui->stackedWidget->setCurrentWidget(m_createWalletWidget);
                m_createWalletWidget->setOfflineWalletInitStep(filePath);
                break;
            case WalletManager::ColdSign:
                //TODO: set up view for offline tx sign
                break;
            case WalletManager::HotSign:
                //TODO: set up view online tx sign and broadcast
                break;
            default:
                enforceOnlineRole();
                ui->stackedWidget->setCurrentWidget(m_walletWidget);
                m_walletWidget->setCurrentWalletPath(filePath);
                break;
            }
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
    event->accept();
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
    PromoteReattachDialog d(this);
    d.setPromoteView();
    d.exec();
}

void MainWindow::reattachActionTriggered()
{
    PromoteReattachDialog d(this);
    d.setReattachView();
    d.exec();
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

void MainWindow::openWalletButtonClicked()
{
    checkDeviceRole();

    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open Wallet File"),
                                                QStandardPaths::standardLocations(
                                                    QStandardPaths::DocumentsLocation).at(0),
                                                tr("IOTAcooler wallet (*.icwl)"));

    if (!file.isEmpty()) {
        openWallet(file);
    }
}

void MainWindow::newWalletWizardCancelled()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::walletCreationCompleted()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::closeWallet()
{
    ui->stackedWidget->setCurrentIndex(0);
    m_walletManager->lockWallet();
}

void MainWindow::showStatusMessage(const QString &message)
{
    ui->statusBar->showMessage(message);
}

void MainWindow::loadWidgets()
{
    m_createWalletWidget = new CreateWalletWizard(this);
    ui->stackedWidget->addWidget(m_createWalletWidget);
    m_walletWidget = new WalletWidget(this);
    ui->stackedWidget->addWidget(m_walletWidget);
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
}

void MainWindow::loadSettings()
{
    SettingsManager sm(this);

    //default node
    UtilsIOTA::currentNodeUrl = sm.getDefaultIOTANodeUrl();
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
