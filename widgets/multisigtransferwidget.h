/**
  * \class MultisigTransferWidget
  * \brief Stacked widget with step-by-step instructions
  * for multisig transfer creation, with online and offline tasks.
  * \author Oirio Joshi
  * \date 2018-03-20
  */

#ifndef MULTISIGTRANSFERWIDGET_H
#define MULTISIGTRANSFERWIDGET_H

#include <QWidget>

namespace Ui {
class MultisigTransferWidget;
}

class MultisigTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MultisigTransferWidget(QWidget *parent = 0);
    ~MultisigTransferWidget();

    /** Setup UI for new transfer */
    void prepareNewTransfer();

signals:
    /** Unfinished transfer was aborted */
    void transferCancelled();

private slots:
    void nextPage();

private:
    Ui::MultisigTransferWidget *ui;
};

#endif // MULTISIGTRANSFERWIDGET_H
