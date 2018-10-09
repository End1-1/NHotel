#include "dlghdmmap.h"
#include "ui_dlghdmmap.h"
#include <QMessageBox>

dlgHdmMap::dlgHdmMap(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgHdmMap)
{
    ui->setupUi(this);
}

dlgHdmMap::~dlgHdmMap()
{
    delete ui;
}

bool dlgHdmMap::TaxMap(QString &tax, QString &program, QString &name)
{
    bool result = false;
    dlgHdmMap *d = new dlgHdmMap(0);
    d->ui->leHDM->setText(tax);
    d->ui->leProgram->setText(program);
    d->ui->leName->setText(name);
    if (d->exec() == QDialog::Accepted) {
        tax = d->ui->leHDM->text();
        program = d->ui->leProgram->text();
        name = d->ui->leName->text();
        result = true;
    }
    delete d;
    return result;
}

void dlgHdmMap::on_btnCancel_clicked()
{
    reject();
}

void dlgHdmMap::on_btnOK_clicked()
{
    ui->leHDM->setText(ui->leHDM->text().trimmed());
    ui->leProgram->setText(ui->leProgram->text().trimmed());
    if (!ui->leHDM->text().length()) {
        QMessageBox::critical(this, tr("Error"), tr("Length of tax is zero"));
        return;
    }
    if (!ui->leProgram->text().length()) {
        QMessageBox::critical(this, tr("Error"), tr("Length of program is zero"));
        return;
    }
    accept();
}
