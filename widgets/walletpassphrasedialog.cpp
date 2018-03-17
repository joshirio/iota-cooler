#include "walletpassphrasedialog.h"
#include "ui_walletpassphrasedialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>

WalletPassphraseDialog::WalletPassphraseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WalletPassphraseDialog)
{
    ui->setupUi(this);

    m_okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setEnabled(false);

    connect(ui->passLineEdit, &QLineEdit::textChanged,
            this, &WalletPassphraseDialog::updateOkButtonState);
}

WalletPassphraseDialog::~WalletPassphraseDialog()
{
    delete ui;
}

QString WalletPassphraseDialog::getWalletPassphrase()
{
    return ui->passLineEdit->text();
}

void WalletPassphraseDialog::updateOkButtonState()
{
    m_okButton->setEnabled(!ui->passLineEdit->text().isEmpty());
}
