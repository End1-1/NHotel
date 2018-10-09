#include "dlggoods.h"
#include "ui_dlggoods.h"

DlgGoods::DlgGoods(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgGoods)
{
    ui->setupUi(this);
}

DlgGoods::~DlgGoods()
{
    delete ui;
}
