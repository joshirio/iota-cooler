/**
  * \class WalletRawDataDialog
  * \brief Dialog to show raw wallet json data for debugging purposes
  * \author Oirio Joshi
  * \date 2018-03-21
  */

#ifndef WALLETRAWDATADIALOG_H
#define WALLETRAWDATADIALOG_H

#include <QDialog>

namespace Ui {
class WalletRawDataDialog;
}

class WalletRawDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WalletRawDataDialog(QWidget *parent = 0);
    ~WalletRawDataDialog();

private:
    Ui::WalletRawDataDialog *ui;
};

#endif // WALLETRAWDATADIALOG_H
