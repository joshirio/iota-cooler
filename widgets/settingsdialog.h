/**
  * \class SettingsDialog
  * \brief Dialog for application settings
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class SettingsManager;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void defaultNodeButtonClicked();
    void nodeLineEditEdited();
    void deviceRoleComboBoxChanged(int index);
    void updatesComboBoxChanged(int index);

private:
    void createConnections();
    void loadSettings();

    Ui::SettingsDialog *ui;
    SettingsManager *sm;
};

#endif // SETTINGSDIALOG_H
