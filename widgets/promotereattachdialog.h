/**
  * \class PromoteReattachDialog
  * \brief Dialog for transactions promoting and reattaching
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef PROMOTEREATTACHDIALOG_H
#define PROMOTEREATTACHDIALOG_H

#include <QDialog>
#include "../components/abstracttangleapi.h"

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
    void reattachLineEditChanged();
    void reattachButtonClicked();
    void tangleAPIRequestFinished(AbstractTangleAPI::RequestType request,
                                  const QString &message);
    void tangleAPIRequestError(AbstractTangleAPI::RequestType request,
                               const QString &message);

private:
    void promoteFinishedViewState(const QString &result);
    void reattachFinishedViewState(const QString &result);

    Ui::PromoteReattachDialog *ui;
    AbstractTangleAPI *m_tangleAPI;
    int m_promoteCounter; /**< Promoting counter, usually 5 txs */
    QString m_currentPromoteTxHash; /** Cached tx hash for multiple promotes */
};

#endif // PROMOTEREATTACHDIALOG_H
