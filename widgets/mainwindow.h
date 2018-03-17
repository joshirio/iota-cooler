/**
  * \class MainWindow
  * \brief The main window of the application
  * \author Oirio Joshi
  * \date 2018-03-04
  */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class CreateWalletWizard;
class WalletManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief Open the specified wallet file and check for next steps
     * (wallet operation) to setup the appropriate view
     * @param filePath -  wallet file path
     */
    void openWallet(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void aboutQtActionTriggered();
    void aboutActionTriggered();
    void helpActionTriggered();
    void settingsActionTriggered();
    void promoteActionTriggered();
    void reattachActionTriggered();
    void newWalletButtonClicked();
    void openWalletButtonClicked();
    void newWalletWizardCancelled();

private:
    void loadWidgets();
    void createMenus();
    void createConnections();
    void loadSettings();

    /**
     * @brief Ask user if the device is a online or offline signer
     * and store/restore choice
     */
    void checkDeviceRole();

    Ui::MainWindow *ui;

    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QMenu *m_toolsMenu;
    QMenu *m_helpMenu;
    QAction *m_quitAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
    QAction *m_settingsAction;
    QAction *m_promoteAction;
    QAction *m_reattachAction;
    QAction *m_helpAction;
    CreateWalletWizard *m_createWalletWidget;
    WalletManager *m_walletManager;
    QString m_currentWalletFilePath;
};

#endif // MAINWINDOW_H
