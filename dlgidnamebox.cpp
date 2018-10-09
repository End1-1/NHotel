#include "dlgidnamebox.h"
#include "ui_dlgidnamebox.h"

DlgIdNameBox::DlgIdNameBox(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::DlgIdNameBox)
{
    ui->setupUi(this);
    ui->leCode->setMaximumWidth(70);
    ui->tblData->setColumnWidth(0, 70);
    ui->leCode->installEventFilter(this);
    ui->leName->installEventFilter(this);
    show();
    qApp->processEvents();
    ui->tblData->setColumnWidth(1, ui->tblData->width() - 75);
}

void DlgIdNameBox::setFilter(const QStringList &code, const QStringList &name)
{
    for (int i = 0; i < code.count(); i++)
        m_values[code[i]] = name[i];
}

void DlgIdNameBox::setName(const QString &name)
{
    setFilterString(name, ui->leName);
}

void DlgIdNameBox::setCode(const QString &id)
{
    setFilterString(id, ui->leCode);
}

DlgIdNameBox::~DlgIdNameBox()
{
    delete ui;
}

void DlgIdNameBox::accept()
{
    QModelIndexList idx = ui->tblData->selectionModel()->selectedIndexes();
    if (!idx.count())
        return;
    m_result.append(ui->tblData->item(idx.at(0).row(), 0)->data(Qt::DisplayRole).toString());
    m_result.append(ui->tblData->item(idx.at(0).row(), 1)->data(Qt::DisplayRole).toString());
    QDialog::accept();
}

void DlgIdNameBox::keyPressEvent(QKeyEvent *e)
{
    QModelIndexList idx = ui->tblData->selectionModel()->selectedIndexes();
    int curRow = -1;
    if (idx.count())
        curRow = idx.at(0).row();
    switch (e->key()) {
    case Qt::Key_Down:
        if (curRow < ui->tblData->rowCount() - 1)
            curRow++;
        ui->tblData->selectRow(curRow);
        break;
    case Qt::Key_Up:
        if (curRow > 0)
            curRow--;
        if (curRow < 0)
            curRow = 0;
        ui->tblData->selectRow(curRow);
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        accept();
        break;
    }
    QDialog::keyPressEvent(e);
}

void DlgIdNameBox::setFilterString(const QString &str, QLineEdit *w)
{
    w->setText(str);
    w->setFocus();
    w->setCursorPosition(str.length());
    filter();
}

void DlgIdNameBox::filter()
{
    QString code = ui->leCode->text().trimmed();
    QString name = ui->leName->text().trimmed();
    QMap<QString, QString> tempValues;
    for (QMap<QString, QString>::const_iterator it = m_values.begin(); it != m_values.end(); it++) {
        bool append = true;
        if (code.length())
            append = append && it.key().startsWith(code, Qt::CaseInsensitive);
        if (name.length())
            append = append && it.value().startsWith(name, Qt::CaseInsensitive);
        if (append)
            tempValues[it.key()] = it.value();
    }
    ui->tblData->clear();
    ui->tblData->setRowCount(tempValues.count());
    int i = 0;
    for (QMap<QString, QString>::const_iterator it = tempValues.begin(); it != tempValues.end(); it++) {
        QTableWidgetItem *itemCode = new QTableWidgetItem(it.key());
        ui->tblData->setItem(i, 0, itemCode);
        QTableWidgetItem *itemName = new QTableWidgetItem(it.value());
        ui->tblData->setItem(i++, 1, itemName);
    }
}

void DlgIdNameBox::on_leName_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    filter();
}

void DlgIdNameBox::on_leCode_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    filter();
}

void DlgIdNameBox::on_tblData_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    m_result.append(ui->tblData->item(index.row(), 0)->data(Qt::DisplayRole).toString());
    m_result.append(ui->tblData->item(index.row(), 1)->data(Qt::DisplayRole).toString());
    accept();
}
