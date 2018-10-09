#include "dedit.h"
#include "ui_dedit.h"

DEdit::DEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DEdit)
{
    ui->setupUi(this);
    ui->leFour->setValidator(new QIntValidator());
    ui->lb1->setText("");
    ui->lb2->setText("");
    ui->lb3->setText("");
    ui->lb4->setText("");
    ui->lb5->setText("");
}

DEdit::~DEdit()
{
    delete ui;
}

void DEdit::setCaptions(const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5)
{
    ui->lb1->setText(s1);
    ui->lb2->setText(s2);
    ui->lb3->setText(s3);
    ui->lb4->setText(s4);
    ui->lb5->setText(s5);
}

void DEdit::setData(const QString &v1, const QString &v2, const QString &v3, const QString &v4, const QString &v5)
{
    ui->leOne->setText(v1);
    ui->leTwo->setText(v2);
    ui->leTree->setText(v3);
    ui->leFour->setText(v4);
    ui->leFive->setText(v5);
}

void DEdit::getData(QString &v1, QString &v2, QString &v3, QString &v4, QString &v5)
{
    v1 = ui->leOne->text();
    v2 = ui->leTwo->text();
    v3 = ui->leTree->text();
    v4 = ui->leFour->text();
    v5 = ui->leFive->text();
}

void DEdit::on_btnCancel_clicked()
{
    reject();
}

void DEdit::on_btnOK_clicked()
{
    accept();
}
