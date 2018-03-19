/**
  * \class WalletWidget
  * \brief Stacked widget for main walllet view and other related actions
  * like addresses list and other.
  * \author Oirio Joshi
  * \date 2018-03-19
  */

#ifndef WALLETWIDGET_H
#define WALLETWIDGET_H

#include <QWidget>

namespace Ui {
class WalletWidget;
}

class WalletWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WalletWidget(QWidget *parent = 0);
    ~WalletWidget();

    /**
     * @brief Set the current wallet file path
     * @param walletFilePath - wallet file
     */
    void setCurrentWalletPath(const QString &walletFilePath);

private:
    Ui::WalletWidget *ui;
    QString m_currentWalletFilePath;
};

#endif // WALLETWIDGET_H
