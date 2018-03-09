/**
  * \class PromoteReattachDialog
  * \brief Dialog for transactions promoting and reattaching
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef PROMOTEREATTACHDIALOG_H
#define PROMOTEREATTACHDIALOG_H

#include <QDialog>

namespace Ui {
class PromoteReattachDialog;
}

class PromoteReattachDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PromoteReattachDialog(QWidget *parent = 0);
    ~PromoteReattachDialog();

    /**
     * @brief Set current up dialog mode to transaction promoting
     */
    void setPromoteView();

    /**
     * @brief Set current up dialog mode to transaction reattaching
     */
    void setReattachView();

private slots:
    void okButtonClicked();
    void cancelButtonClicked();
    void promoteLineEditChanged();
    void promoteButtonClicked();

private:
    Ui::PromoteReattachDialog *ui;
};

#endif // PROMOTEREATTACHDIALOG_H
