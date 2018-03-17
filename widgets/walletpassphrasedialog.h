/**
  * \class WalletPassphraseDialog
  * \brief Dialog for wallet password input
  * \author Oirio Joshi
  * \date 2018-03-17
  */

#ifndef WALLETPASSPHRASEDIALOG_H
#define WALLETPASSPHRASEDIALOG_H

#include <QDialog>

class QPushButton;

namespace Ui {
class WalletPassphraseDialog;
}

class WalletPassphraseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WalletPassphraseDialog(QWidget *parent = 0);
    ~WalletPassphraseDialog();

    /**
     * @brief Get the entered wallet passphrase
     * @return the entered password
     */
    QString getWalletPassphrase();

private slots:
    void updateOkButtonState();

private:
    Ui::WalletPassphraseDialog *ui;
    QPushButton *m_okButton;
};

#endif // WALLETPASSPHRASEDIALOG_H
