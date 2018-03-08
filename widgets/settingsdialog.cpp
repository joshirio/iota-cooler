#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "../utils/definitionholder.h"
#include "../components/settingsmanager.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    sm = new SettingsManager(this);

    createConnections();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::defaultNodeButtonClicked()
{
    ui->nodeLineEdit->setText(DefinitionHolder::DEFAULT_NODE);
    nodeLineEditEdited();
}

void SettingsDialog::nodeLineEditEdited()
{
    QString url = ui->nodeLineEdit->text().trimmed();
    if (url.contains("://")) {
        sm->setDefaultIOTANodeUrl(url);
    } else {
        ui->nodeLineEdit->undo();
    }
}

void SettingsDialog::createConnections()
{
    connect(ui->defaultNodeButton, &QPushButton::clicked,
            this, &SettingsDialog::defaultNodeButtonClicked);
    connect(ui->nodeLineEdit, &QLineEdit::editingFinished,
            this, &SettingsDialog::nodeLineEditEdited);
}

void SettingsDialog::loadSettings()
{
    QString nodeUrl = sm->getDefaultIOTANodeUrl();
    ui->nodeLineEdit->setText(nodeUrl);
}
