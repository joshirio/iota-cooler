#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "promotereattachdialog.h"
#include "../utils/definitionholder.h"

#include <QtWidgets/QMenuBar>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_menuBar = ui->menuBar;
    ui->mainToolBar->hide();
    ui->openWalletButton->setFocus();

    createMenus();
    createConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
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
}
