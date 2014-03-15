#include "preferencedialog.h"
#include "ui_preferencedialog.h"

#include "common.h"

PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);
    ui->nameEdit->setText(g_settings->value(SETTINGS_NAME).toString());
    ui->iplistWidget->addItems(g_settings->value(SETTINGS_ALLOW_IPS).toStringList());
    ui->uuidEdit->setText(g_settings->value(SETTINGS_UUID).toString());
    ui->pskEdit->setText(g_settings->value(SETTINGS_PRESHAREDKEY).toString());
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

void PreferenceDialog::accept()
{
    g_settings->setValue(SETTINGS_NAME, ui->nameEdit->text());
    //g_settings->setValue(SETTINGS_ALLOW_IPS, ui->iplistWidget->);
    QDialog::accept();
}
