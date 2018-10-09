#include "dlgpayment.h"
#include "ui_dlgpayment.h"
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "dlgidnamebox.h"

QStringList DlgPayment::m_itemCode;
QStringList DlgPayment::m_itemName;

void DlgPayment::count(float &f1, float &f2, float &f3)
{
    float total = f1 + f2 + f3;
    float store = f2 + f3;
    if (m_total < total) {
        float diff = total - m_total;
        if (diff > store)
            diff = store;
        if (diff > f2) {
            diff -= f2;
            f2 = 0;
        } else {
            f2 -= diff;
            diff = 0;
        }
        if (diff > 0.01) {
            if (diff > f3) {
                diff -= f3;
                f3 = 0;
            } else {
                f3 -= diff;
                diff = 0;
            }
        }
        if (f1 > m_total)
            f1 = m_total;
    }
    if (f1 + f2 + f3 < m_total)
        f2 += m_total - f1 - f2 - f3;
}

void DlgPayment::setValues(float cash, float credit, float prepayment)
{
    ui->leCash->setText(QString::number(cash, 'f', 0));
    ui->leCreditCard->setText(QString::number(credit, 'f', 0));
    ui->lePrepaiment->setText(QString::number(prepayment, 'f', 0));
}

void DlgPayment::disconnectAll()
{
    ui->leCash->disconnect();
    ui->leCreditCard->disconnect();
    ui->lePrepaiment->disconnect();
}

void DlgPayment::connectAll()
{
    connect(ui->leCash, SIGNAL(returnPressed()), this, SLOT(on_leCash_returnPressed()));
    connect(ui->leCash, SIGNAL(textChanged(QString)), this, SLOT(on_leCash_textChanged(QString)));
    connect(ui->leCreditCard, SIGNAL(textChanged(QString)), this, SLOT(on_leCreditCard_textChanged(QString)));
    connect(ui->leCreditCard, SIGNAL(returnPressed()), this, SLOT(on_leCreditCard_returnPressed()));
    connect(ui->lePrepaiment, SIGNAL(textChanged(QString)), this, SLOT(on_lePrepaiment_textChanged(QString)));
    connect(ui->lePrepaiment, SIGNAL(returnPressed()), this, SLOT(on_lePrepaiment_returnPressed()));

}

void DlgPayment::getCreditAmount(float &total, float &card, float &prepaiment)
{
    total = ui->leCash->text().toFloat();
    card = ui->leCreditCard->text().toFloat();
    prepaiment = ui->lePrepaiment->text().toFloat();
}

DlgPayment::DlgPayment(float amount, QWidget *parent) :
    QDialog(parent, Qt::WindowStaysOnTopHint),
    ui(new Ui::DlgPayment)
{
    ui->setupUi(this);
    QList<int> colWidths;
    colWidths << 100 << 250 << 80 << 80 << 80 << 80;
    for (int i = 0; i < ui->tblGoods->columnCount(); i++)
        ui->tblGoods->setColumnWidth(i, colWidths[i]);
    m_timeLeft = 7;
    m_total = amount;
    ui->leTotal->setText(QString::number(amount, 'f', 2));
    ui->leTotalGoods->setText(ui->leTotal->text());
    ui->leCash->setText(QString::number(amount, 'f', 2));
    ui->leCreditCard->setValidator(new QDoubleValidator(0, amount, 2));
    ui->leCreditCard->setFocus();
    ui->lePrepaiment->setValidator(new QDoubleValidator(0, amount, 2));
    ui->leCash->setValidator(new QDoubleValidator(0, amount, 2));
    ui->leCreditCard->setValidator(new QDoubleValidator(0, amount, 2));
    m_textSelected = true;
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    connect(ui->leCreditCard, SIGNAL(focus(bool)), this, SLOT(editFocus(bool)));
    connect(ui->lePrepaiment, SIGNAL(focus(bool)), this, SLOT(editFocus(bool)));
    connect(ui->leCash, SIGNAL(focus(bool)), this, SLOT(editFocus(bool)));
    if (!m_itemCode.count()) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QIBASE", "blabla");
        QSettings s("TaxPrint", "TaxPrint");
        db.setDatabaseName(s.value("convertdb").toString());
        db.setUserName("sysdba");
        db.setPassword("masterkey");
        db.setConnectOptions("lc_ctype=utf-8");
        db.open();
        QSqlQuery q(db);
        q.exec("select armsoft_id, name_arm from convert order by name_arm");
        while (q.next()) {
            m_itemCode.append(q.value(0).toString());
            m_itemName.append(q.value(1).toString());
        }
        db.close();
    }
    QSqlDatabase::removeDatabase("blabla");
    m_timer.start(1000);
}

void DlgPayment::setGoods(QList<Goods> &goods)
{
    ui->tblGoods->setRowCount(goods.count());
    for (int i = 0; i < goods.count(); i++) {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setData(Qt::EditRole, goods[i].armsoftId);
        ui->tblGoods->setItem(i, 0, item);
        item = new QTableWidgetItem();
        item->setData(Qt::EditRole, goods[i].armName);
        ui->tblGoods->setItem(i, 1, item);
        item = new QTableWidgetItem();
        item->setData(Qt::EditRole, goods[i].qty);
        ui->tblGoods->setItem(i, 2, item);
        item = new QTableWidgetItem();
        item->setData(Qt::EditRole, goods[i].price);
        ui->tblGoods->setItem(i, 3, item);
        item = new QTableWidgetItem();
        item->setData(Qt::EditRole, goods[i].qty * goods[i].price);
        ui->tblGoods->setItem(i, 4, item);
        item = new QTableWidgetItem();
        item->setData(Qt::EditRole, goods[i].dept);
        ui->tblGoods->setItem(i, 5, item);
    }
}

void DlgPayment::getGoods(QList<Goods> &goods)
{
    goods.clear();
    for (int i = 0; i < ui->tblGoods->rowCount(); i++) {
        Goods g;
        g.armsoftId = ui->tblGoods->item(i, 0)->data(Qt::EditRole).toString();
        g.armName = ui->tblGoods->item(i, 1)->data(Qt::EditRole).toString();
        g.qty = ui->tblGoods->item(i, 2)->data(Qt::EditRole).toFloat();
        g.price = ui->tblGoods->item(i, 3)->data(Qt::EditRole).toFloat();
        g.dept = ui->tblGoods->item(i, 5)->data(Qt::EditRole).toInt();
        goods.append(g);
    }
}

DlgPayment::~DlgPayment()
{
    delete ui;
}

void DlgPayment::leQtyChanged(const QString &arg1)
{
    QModelIndexList idx = ui->tblGoods->selectionModel()->selectedIndexes();
    ui->tblGoods->item(idx.at(0).row(), idx.at(0).column())->setData(Qt::EditRole, arg1.toDouble());
    ui->tblGoods->item(idx.at(0).row(), 4)->setData(Qt::EditRole, arg1.toDouble() * ui->tblGoods->item(idx.at(0).row(), 3)->data(Qt::EditRole).toDouble());
    countAmount();
}

void DlgPayment::lePriceChanged(const QString &arg1)
{
    QModelIndexList idx = ui->tblGoods->selectionModel()->selectedIndexes();
    ui->tblGoods->item(idx.at(0).row(), idx.at(0).column())->setData(Qt::EditRole, arg1.toDouble());
    ui->tblGoods->item(idx.at(0).row(), 4)->setData(Qt::EditRole, arg1.toDouble() * ui->tblGoods->item(idx.at(0).row(), 2)->data(Qt::EditRole).toDouble());
    countAmount();
}

void DlgPayment::leAmountChanged(const QString &arg1)
{
    QModelIndexList idx = ui->tblGoods->selectionModel()->selectedIndexes();
    ui->tblGoods->item(idx.at(0).row(), idx.at(0).column())->setData(Qt::EditRole, arg1.toDouble());
    if (ui->tblGoods->item(idx.at(0).row(), 2)->data(Qt::EditRole).toDouble() > 0.01)
        ui->tblGoods->item(idx.at(0).row(), 3)->setData(Qt::EditRole, arg1.toDouble() / ui->tblGoods->item(idx.at(0).row(), 2)->data(Qt::EditRole).toDouble());
    else
        ui->tblGoods->item(idx.at(0).row(), 3)->setData(Qt::EditRole, 0);
    countAmount();
}

void DlgPayment::leNameChanged(const QString &arg1)
{
    DlgIdNameBox *dlg = new DlgIdNameBox(this);
    dlg->setFilter(m_itemCode, m_itemName);
    dlg->setName(arg1);
    if (dlg->exec() == QDialog::Accepted) {
        QModelIndexList idx =  ui->tblGoods->selectionModel()->selectedIndexes();
        if (idx.count()) {
            ui->tblGoods->item(idx.at(0).row(), 0)->setData(Qt::EditRole, dlg->m_result.at(0));
            ui->tblGoods->item(idx.at(0).row(), 1)->setData(Qt::EditRole, dlg->m_result.at(1));
            QLineEdit *l = static_cast<QLineEdit*>(ui->tblGoods->cellWidget(idx.at(0).row(), idx.at(0).column()));
            if (l) {
                l->disconnect();
                l->setText(dlg->m_result.at(1));
                connect(l, SIGNAL(textChanged(QString)), this, SLOT(leNameChanged(QString)));
            }
        }
    }
    delete dlg;
}

void DlgPayment::leCodeChanged(const QString &arg1)
{
    DlgIdNameBox *dlg = new DlgIdNameBox(this);
    dlg->setFilter(m_itemCode, m_itemName);
    dlg->setCode(arg1);
    if (dlg->exec() == QDialog::Accepted) {
        QModelIndexList idx =  ui->tblGoods->selectionModel()->selectedIndexes();
        if (idx.count()) {
            ui->tblGoods->item(idx.at(0).row(), 0)->setData(Qt::EditRole, dlg->m_result.at(0));
            ui->tblGoods->item(idx.at(0).row(), 1)->setData(Qt::EditRole, dlg->m_result.at(1));
            QLineEdit *l = static_cast<QLineEdit*>(ui->tblGoods->cellWidget(idx.at(0).row(), idx.at(0).column()));
            if (l) {
                l->disconnect();
                l->setText(dlg->m_result.at(0));
                connect(l, SIGNAL(textChanged(QString)), this, SLOT(leCodeChanged(QString)));
            }
        }
    }
    delete dlg;
}

void DlgPayment::on_leCreditCard_textChanged(const QString &arg1)
{
    disconnectAll();
    float cash = ui->leCash->text().toFloat();
    float credit = arg1.toFloat();
    float prepayment = ui->lePrepaiment->text().toFloat();
    count(credit, cash, prepayment);
    setValues(cash, credit, prepayment);
    m_timer.stop();
    connectAll();
}

void DlgPayment::on_leCreditCard_returnPressed()
{
    ui->lePrepaiment->setFocus();
}

void DlgPayment::timerTimeout()
{
    ui->lbAutoAccept->setText(QString("%1 %2 %3").arg(tr("Automatically accept in ")).arg(m_timeLeft--).arg(tr("seconds")));
    if (m_timeLeft < 0) {
        m_timer.stop();
        accept();
    }
}

void DlgPayment::editFocus(bool v)
{
    if (v) {
        m_currentEdit = static_cast<FLineEdit*>(sender());
        m_currentEdit->setSelection(0, m_currentEdit->text().length());
    }
}

void DlgPayment::on_pushButton_12_clicked()
{
    ui->leCreditCard->clear();
}

void DlgPayment::on_pushButton_clicked()
{
    text(sender());
}

void DlgPayment::text(QObject *o)
{
    QPushButton *b = static_cast<QPushButton*>(o);
    QString t = m_currentEdit->text();
    if (m_textSelected) {
        m_textSelected = false;
        t = "";
    }
    QString s = b->text();
    if (s == ".")
        if (m_currentEdit->text().contains("."))
            s = "";
    m_currentEdit->setText(t + s);
}

void DlgPayment::countAmount()
{
    disconnectAll();
    m_total = 0;
    for (int i = 0; i < ui->tblGoods->rowCount(); i++)
        m_total += ui->tblGoods->item(i, 4)->data(Qt::EditRole).toDouble();
    ui->leTotal->setText(QString::number(m_total, 'f', 2));
    ui->leTotalGoods->setText(QString::number(m_total, 'f', 2));
    ui->leCash->setText(QString::number(m_total, 'f', 2));
    ui->leCreditCard->setText("0");
    ui->lePrepaiment->setText("0");
    connectAll();
}

void DlgPayment::on_pushButton_4_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_7_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_2_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_5_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_8_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_3_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_6_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_9_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_10_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_11_clicked()
{
    text(sender());
}

void DlgPayment::on_pushButton_13_clicked()
{
    accept();
}

FLineEdit::FLineEdit(QWidget *parent) :
    QLineEdit(parent)
{

}

void FLineEdit::focusInEvent(QFocusEvent *e)
{
    emit focus(true);
    QLineEdit::focusInEvent(e);
}

void FLineEdit::focusOutEvent(QFocusEvent *e)
{
    emit focus(false);
    QLineEdit::focusOutEvent(e);
}

void DlgPayment::on_lePrepaiment_textChanged(const QString &arg1)
{
    disconnectAll();
    float cash = ui->leCash->text().toFloat();
    float credit = ui->leCreditCard->text().toFloat();
    float prepayment = arg1.toFloat();
    count(prepayment, cash, credit);
    setValues(cash, credit, prepayment);
    m_timer.stop();
    connectAll();
}

void DlgPayment::on_btnCancel_clicked()
{
    reject();
}

void DlgPayment::on_lePrepaiment_returnPressed()
{
    accept();
}

void DlgPayment::on_leCash_textChanged(const QString &arg1)
{
    disconnectAll();
    float cash = arg1.toFloat();
    float credit = ui->leCreditCard->text().toFloat();
    float prepayment = ui->lePrepaiment->text().toFloat();
    count(cash, credit, prepayment);
    setValues(cash, credit, prepayment);
    m_timer.stop();
    connectAll();
}

void DlgPayment::on_leCash_returnPressed()
{
    ui->leCreditCard->setFocus();
}

void DlgPayment::on_btnDelRow_clicked()
{
    QModelIndexList rows = ui->tblGoods->selectionModel()->selectedIndexes();
    if (!rows.count())
        return;
    if (QMessageBox::question(this, tr("Qeustion"), tr("Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;
    ui->tblGoods->removeRow(rows.at(0).row());
    countAmount();
}

void DlgPayment::on_tblGoods_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (previousColumn > -1 && previousRow > -1) {
        QWidget *oldWidget = ui->tblGoods->cellWidget(previousRow, previousColumn);
        if (oldWidget) {
            ui->tblGoods->removeCellWidget(previousRow, previousColumn);
            oldWidget->deleteLater();
        }
    }
    if (currentRow > -1 && currentColumn > -1) {
        switch (currentColumn) {
        case 0: {
            QLineEdit *l = new QLineEdit();
            l->setText(ui->tblGoods->item(currentRow, currentColumn)->data(Qt::EditRole).toString());
            l->setFrame(false);
            l->setSelection(0, l->text().length());
            connect(l, SIGNAL(textChanged(QString)), this, SLOT(leCodeChanged(QString)));
            ui->tblGoods->setCellWidget(currentRow, currentColumn, l);
            break;
        }
        case 1: {
            QLineEdit *l = new QLineEdit();
            l->setText(ui->tblGoods->item(currentRow, currentColumn)->data(Qt::EditRole).toString());
            l->setFrame(false);
            l->setSelection(0, l->text().length());
            connect(l, SIGNAL(textChanged(QString)), this, SLOT(leNameChanged(QString)));
            ui->tblGoods->setCellWidget(currentRow, currentColumn, l);
            break;
        }
        case 2: {
            QLineEdit *l = new QLineEdit();
            l->setValidator(new QDoubleValidator());
            l->setText(ui->tblGoods->item(currentRow, currentColumn)->data(Qt::EditRole).toString());
            l->setFrame(false);
            l->setSelection(0, l->text().length());
            connect(l, SIGNAL(textChanged(QString)), this, SLOT(leQtyChanged(QString)));
            ui->tblGoods->setCellWidget(currentRow, currentColumn, l);
            break;
        }
        case 3: {
            QLineEdit *l = new QLineEdit();
            l->setValidator(new QDoubleValidator());
            l->setText(ui->tblGoods->item(currentRow, currentColumn)->data(Qt::EditRole).toString());
            l->setFrame(false);
            l->setSelection(0, l->text().length());
            connect(l, SIGNAL(textChanged(QString)), this, SLOT(lePriceChanged(QString)));
            ui->tblGoods->setCellWidget(currentRow, currentColumn, l);
            break;
        }
        case 4: {
            QLineEdit *l = new QLineEdit();
            l->setValidator(new QDoubleValidator());
            l->setText(ui->tblGoods->item(currentRow, currentColumn)->data(Qt::EditRole).toString());
            l->setFrame(false);
            l->setSelection(0, l->text().length());
            connect(l, SIGNAL(textChanged(QString)), this, SLOT(leAmountChanged(QString)));
            ui->tblGoods->setCellWidget(currentRow, currentColumn, l);
            break;
        }
        default:
            break;
        }
    }
}

void DlgPayment::on_btnAddRow_clicked()
{
    int row = ui->tblGoods->rowCount();
    ui->tblGoods->setRowCount(row + 1);
    for (int i = 0; i < ui->tblGoods->columnCount(); i++)
        ui->tblGoods->setItem(row, i, new QTableWidgetItem());
}

void DlgPayment::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index)
    m_timer.stop();
}
