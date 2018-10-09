#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QProcess>
#include "dedit.h"
#include "dlghdmmap.h"
#include <QMenu>
#include <QDateTime>
#include "dlgpayment.h"
#include <QShortcut>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->leAmountPrepayment->setValidator(new QDoubleValidator(0, 100000000, 2));
    ui->lePrepaymentCard->setValidator(new QDoubleValidator(0, 100000000, 2));
    ui->leQty->setValidator(new QDoubleValidator(0, 10000, 3));
    ui->lePrice->setValidator(new QDoubleValidator(0, 1000000000, 2));
    QList<int> colWidths;
    colWidths << 120 << 250 << 120 << 120 << 120 << 120;
    for (int i = 0; i < ui->tblGoodsList->columnCount(); i++)
        ui->tblGoodsList->setColumnWidth(i, colWidths.at(i));

    loadSettings();
    m_fb = QSqlDatabase::addDatabase("QIBASE", "qibase");
    m_fb.setDatabaseName(ui->leConvertDb->text());
    m_fb.setUserName("sysdba");
    m_fb.setPassword("masterkey");
    m_fb.setConnectOptions("lc_ctype=utf-8");

    reloadMaps();
    initDb();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    //connect(m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

    QShortcut *f12 = new QShortcut(this);
    f12->setKey(QKeySequence("F12"));
    connect(f12, &QShortcut::activated, [this]() {
        on_btnPrintTaxCheck_clicked();
    });

    QShortcut *del = new QShortcut(this);
    del->setKey(QKeySequence("F11"));
    connect(del, &QShortcut::activated, [this](){
        on_btnClearTaxCheck_clicked();
    });

    ui->leCode->setFocus();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    ui->leDbFile->setText(QFileDialog::getOpenFileName(this, tr("Select db")).replace("/", "\\"));
    saveSettings();
    initDb();
}

void Dialog::timeout()
{
    printTax();
}

void Dialog::initDb()
{

}

void Dialog::loadSettings()
{
    m_appPath = QApplication::applicationDirPath();
    QSettings settings("TaxPrint", "TaxPrint");
    ui->leDbFile->setText(settings.value("db").toString());
    ui->leTaxIP->setText(settings.value("ip").toString());
    ui->leTaxPort->setText(settings.value("port").toString());
    ui->leTaxPassword->setText(settings.value("password").toString());
    ui->leConvertDb->setText(settings.value("convertdb").toString());
    ui->leOrderNumber->setText(settings.value("ordnum").toString());
    ui->chPrintOnlyCurrentDate->setChecked(settings.value("print_current_date").toBool());
    ui->chAutoPrint->setChecked(settings.value("autoprint", false).toBool());
    ui->lePrintTimeout->setText(settings.value("printtimeout", "7").toString());
    ui->lePrintTimeout->setEnabled(!ui->chAutoPrint->isChecked());
    ui->chNOFox->setChecked(settings.value("nofoxdb").toBool());
    ui->leADGCode->setText(settings.value("adgcode").toString());
    ui->chClearCheckAfterPrint->setChecked(settings.value("cleartaxafterprint").toBool());
    QStringList taxMap = settings.value("taxmap").toString().split(";", QString::SkipEmptyParts);
    ui->tblTax->clearContents();
    ui->tblTax->setRowCount(0);
    ui->tblTax->setColumnCount(3);
    ui->tblTax->setColumnWidth(2, 200);
    for (QStringList::const_iterator it = taxMap.begin(); it != taxMap.end(); it++) {
        QStringList tax = (*it).split(":");
        while (tax.count() < 3)
            tax.append("");
        m_taxMap[tax.at(1)] = tax.at(0);
        int row = ui->tblTax->rowCount();
        ui->tblTax->setRowCount(row + 1);
        for (int i = 0; i < ui->tblTax->columnCount(); i++)
            ui->tblTax->setItem(row, i, new QTableWidgetItem(tax.at(i)));
    }
    makeComboTax();
}

void Dialog::saveSettings()
{
    QSettings settings("TaxPrint", "TaxPrint");
    settings.setValue("db", ui->leDbFile->text());
    settings.setValue("ip", ui->leTaxIP->text());
    settings.setValue("port", ui->leTaxPort->text());
    settings.setValue("password", ui->leTaxPassword->text());
    settings.setValue("convertdb", ui->leConvertDb->text());
    settings.setValue("ordnum", ui->leOrderNumber->text());
    settings.setValue("print_current_date", ui->chPrintOnlyCurrentDate->isChecked());
    settings.setValue("autoprint", ui->chAutoPrint->isChecked());
    settings.setValue("printtimeout", ui->lePrintTimeout->text().toInt());
    settings.setValue("nofoxdb", ui->chNOFox->isChecked());
    settings.setValue("adgcode", ui->leADGCode->text());
    settings.setValue("cleartaxafterprint", ui->chClearCheckAfterPrint->isChecked());
    QString taxMap;
    for (int i = 0; i < ui->tblTax->rowCount(); i++)
        taxMap += QString("%1:%2:%3;")
                .arg(ui->tblTax->item(i, 0)->data(Qt::DisplayRole).toString())
                .arg(ui->tblTax->item(i, 1)->data(Qt::DisplayRole).toString())
                .arg(ui->tblTax->item(i, 2)->data(Qt::DisplayRole).toString());
    settings.setValue("taxmap", taxMap);
    reloadMaps();
    initDb();
}

void Dialog::printTax()
{
    if (ui->chNOFox->isChecked()) {
        return;
    }
    m_timer.stop();
    isWork();

    QSqlDatabase m_db = QSqlDatabase::addDatabase("QODBC");
    m_db.setDatabaseName("DRIVER={Microsoft Visual FoxPro Driver};SourceType=DBF;SourceDB=" + ui->leDbFile->text());

    if (!m_db.open()) {
        log(m_db.lastError().databaseText());
        m_db = QSqlDatabase::addDatabase("QODBC");
        m_timer.start();
        return;
    }

    log(tr("Run print, db: ") + ui->leDbFile->text());

    QSqlQuery q(m_db);

    /* Proced the jv - outno
     * if a same jv  have multiple outno
     * jv must be different */
    if (!q.exec("select distinct(" + ui->leOrderNumber->text() + "), OUTNO from " + ui->leDbFile->text())) {
        log("SQL error: " + q.lastError().databaseText());
        m_timer.start();
        return;
    }
    QMap<QString, QString> jvOutNo;
    QMultiMap<QString, QString> jvToUpdate;
    while (q.next()) {
        if (!jvOutNo.contains(q.value(0).toString())) {
            jvOutNo[q.value(0).toString()] = q.value(1).toString();
            continue;
        } else {
            for (QMultiMap<QString, QString>::const_iterator it = jvToUpdate.begin(); it!= jvToUpdate.end(); it++)
                if (!it.key().compare(q.value(0).toString()) && !it.value().compare(q.value(1).toString()))
                    continue;
            jvToUpdate.insert(q.value(0).toString(), q.value(1).toString());
        }
    }
    int ordNum = 1;
    for (QMultiMap<QString, QString>::const_iterator it = jvToUpdate.begin(); it != jvToUpdate.end(); it++) {
        QString newOrdNum = QString("SS%1").arg(ordNum++);
        if (!q.exec("update " + ui->leDbFile->text() + " set " + ui->leOrderNumber->text() + "='" + newOrdNum + "' "
                    " where " + ui->leOrderNumber->text() + "='" + it.key() + "' "
                    " and OUTNO='" + it.value() + "'")) {
            log("SQL error: " + q.lastError().databaseText() + ", " + q.lastQuery());
        }
    }
    /* End of procced the jv - outno */

    QStringList keysToDelete;
    log(tr("Grouping goods"));
    QMap<QString, accGoods> goodsGroups;
    if (!q.exec("select SALCODE, LOCAL, SALD, ENT_DAT, QTY, OUTNO, JV, SERIAL from " + ui->leDbFile->text())) {
        log("SQL error: " + q.lastError().databaseText());
        m_db.close();
        m_timer.start();
        return;
    }
    while (q.next()) {
        QString key = q.value(0).toString().trimmed();
        if (!m_goodsGroups.values().contains(key))
            continue;
        keysToDelete.append(q.value(0).toString());
        QString newKey = m_goodsGroups.key(key);
        if (!goodsGroups.contains(newKey)) {
            accGoods a;
            a.name = q.value(2).toString();
            a.salCode = q.value(0).toString();
            a.entDate = q.value(3).toString();
            a.qty = q.value(4).toDouble();
            a.amount = q.value(1).toDouble();
            a.outNo = q.value(5).toString();
            a.jv = q.value(6).toString();
            a.serial = q.value(7).toString();
            goodsGroups.insert(newKey, a);
        }
        else {
            accGoods &a = goodsGroups[newKey];
            a.qty += q.value(4).toDouble();
            a.amount += q.value(1).toDouble();
        }
    }
    keysToDelete.removeDuplicates();
    for (QStringList::const_iterator it = keysToDelete.begin(); it != keysToDelete.end(); it++)
        q.exec("delete from " + ui->leDbFile->text() + " where salcode='" + *it + "'");

    for (QMap<QString, accGoods>::const_iterator it = goodsGroups.begin(); it != goodsGroups.end(); it++)
        if (!q.exec("insert into " + ui->leDbFile->text() + " (SERIAL, JV, OUTNAME, PAX, DEP_DAT, "
                    "ROOM, SAL, GUEST, "
                    "SALD, SALCODE, ENT_DAT,QTY,LOCAL,OUTNO, IND) " +
            "values ('" + it->serial + "', '" + it->jv + "', '', 0, date(),' ',' ',' ', '"
                    + it->name + "', '" + it.key() + "', date(), "
                    + "1" /*QString::number(it->qty, 'f', 2)*/ +
               ", " + QString::number(it->amount, 'f', 2) + ", '" + it->outNo + "', 1)"))
            log("SQL error: " + q.lastError().databaseText() + ": " + q.lastQuery());

    /* End of grouping foods */

    QString condDate = (ui->chPrintOnlyCurrentDate->isChecked() ? " where ENT_DAT=date() " : "");
    if (!q.exec("select distinct(" + ui->leOrderNumber->text() + ") from " + ui->leDbFile->text() + condDate))
    {
        log("SQL error: " + q.lastError().databaseText());
        m_db.close();
        m_timer.start();
        return;
    }

    QStringList m_orders;
    while (q.next())
        m_orders << q.value(0).toString();

    for (QStringList::const_iterator it = m_orders.begin(); it != m_orders.end(); it++)
    {
        log(tr("Print tax check: ") + *it);
        float total = 0;
        float amountCredit = 0;
        float amountPrepaiment = 0;
        QStringList errorList;
        QList<Goods> orderGoods;
        q.exec("select * from " +  ui->leDbFile->text() + " where SALCODE<>'' AND " + ui->leOrderNumber->text() + "='" + *it + "'");
        while (q.next())
        {
            Goods g;
            if (!m_goodsMap.contains(q.value("SALCODE").toString().trimmed()))
            {
                errorList.append(tr("Goods maps key missing") + ": " + q.value("SALCODE").toString() + "---" + q.value("SALD").toString());
                g.armName = q.value("SALD").toString();
                g.armsoftId = q.value("SALCODE").toString();
            } else {
                g.armName = m_goodsMap[q.value("SALCODE").toString().trimmed()].armName;
                g.armsoftId = m_goodsMap[q.value("SALCODE").toString().trimmed()].armsoftId;
            }
            g.qty = q.value("QTY").toFloat();
            g.price = q.value("LOCAL").toFloat() / g.qty;
            g.dept = m_taxMap[q.value("OUTNO").toString()].toInt();
            orderGoods.append(g);
            total += (g.qty * g.price);
        }

        if (errorList.count())
        {
            for (int i = 0; i < errorList.count(); i++)
                ui->pteLog->appendPlainText(errorList[i] + "\r\n");
            //QMessageBox::critical(this, tr("Print"), tr("Cannot print, becouse some codes is missing in conversion table"));
            //continue;
        }

        QString fileName = QString("tax_%1.json")
                .arg(*it);

        if (!ui->chAutoPrint->isChecked()) {
            DlgPayment *dl = new DlgPayment(total, this);
            dl->setGoods(orderGoods);
            dl->m_timeLeft = ui->lePrintTimeout->text().toInt();
            dl->m_timer.start(1000);
            if (dl->exec() != QDialog::Accepted) {
                delete dl;
                if (!q.exec("delete from " + ui->leDbFile->text() + " where " + ui->leOrderNumber->text() + "='" + *it + "'")) {
                    log(q.lastError().databaseText());
                    log(q.lastQuery());
                }
                m_db.close();
                m_timer.start();
                return;
            }
            dl->getGoods(orderGoods);
            dl->getCreditAmount(total, amountCredit, amountPrepaiment);
            delete dl;
        }

        QFile f(m_appPath + "\\" + fileName);
        if (f.open(QIODevice::WriteOnly)) {
            f.write((QString("{\"seq\":1, "
                             "\"paidAmount\":%1, \"paidAmountCard\":%2, \"partialAmount\":0, \"prePaymentAmount\":%3,"
                             "\"mode\":2, \"useExtPOS\":false, \"items\":[")
                     .arg(QString::number(total, 'f', 2))
                     .arg(QString::number(amountCredit, 'f', 2))
                     .arg(QString::number(amountPrepaiment, 'f', 2))).toUtf8());
            bool first = true;
            for (int i = 0; i < orderGoods.count(); i++) {
                if (!first)
                    f.write(",");
                else
                    first = false;
                f.write(QString("{\"dep\":%1,"
                                "\"qty\":%2,"
                                "\"price\":%3,"
                                "\"productCode\":\"%4\","
                                "\"productName\":\"%5\","
                                "\"unit\":\"%6\","
                                "\"totalPrice\":%7}")
                        .arg(orderGoods[i].dept)
                        .arg(orderGoods[i].qty)
                        .arg(orderGoods[i].price)
                        .arg(orderGoods[i].armsoftId)
                        .arg(orderGoods[i].armName)
                        .arg(QString::fromUtf8("հատ"))
                        .arg(orderGoods[i].qty * orderGoods[i].price).toUtf8());
            }
            f.write("]}");
            f.close();
        }

        if (!q.exec("delete from " + ui->leDbFile->text() + " where " + ui->leOrderNumber->text() + "='" + *it + "'")) {
            log(q.lastError().databaseText());
            log(q.lastQuery());
        }
    }

    m_db.close();

    QStringList args;
    args << ui->leTaxIP->text()
         << ui->leTaxPort->text()
         << ui->leTaxPassword->text()
         << m_appPath;
    QProcess *p = new QProcess();
    p->start(m_appPath + "\\printtax.exe", args);
    m_timer.start();
}

void Dialog::reloadMaps()
{
    if (!m_fb.open())
    {
        QMessageBox::critical(this, tr("Firebird databases error"), m_fb.lastError().databaseText());
        return;
    }

    m_goodsMap.clear();
    QSqlQuery q(m_fb);
    q.exec("select fox_id, armsoft_id, name_arm, price, adgt from convert");
    while (q.next())
    {
        Goods g;
        g.foxId = q.value(0).toString();
        g.armsoftId = q.value(1).toString();
        g.armName = q.value(2).toString();
        g.price = q.value(3).toDouble();
        g.adgt = q.value(4).toString();
        m_goodsMap[g.foxId] = g;
    }

    q.exec("select src, dst from goods_grouping");
    while (q.next()) {
        m_goodsGroups.insert(q.value(0).toString(), q.value(1).toString());
    }

    m_fb.close();

    ui->tblGoodsMap->clearContents();
    ui->tblGoodsMap->setRowCount(m_goodsMap.count());
    QList<int> colW;
    colW << 120 << 120 << 350;
    for (int i = 0; i < colW.count(); i++)
        ui->tblGoodsMap->setColumnWidth(i, colW.at(i));
    int row = 0;
    for (QMap<QString, Goods>::const_iterator it = m_goodsMap.begin(); it != m_goodsMap.end(); it++)
    {
        ui->tblGoodsMap->setItem(row, 0, new QTableWidgetItem(it->foxId));
        ui->tblGoodsMap->setItem(row, 1, new QTableWidgetItem(it->armsoftId));
        ui->tblGoodsMap->setItem(row, 2, new QTableWidgetItem(it->armName));
        ui->tblGoodsMap->setItem(row, 3, new QTableWidgetItem(QString::number(it->price, 'f', 0)));
        ui->tblGoodsMap->setItem(row, 4, new QTableWidgetItem(it->adgt));
        row++;
    }

    ui->tblGoodsGroup->clearContents();
    ui->tblGoodsGroup->setRowCount(m_goodsGroups.count());
    colW.clear();
    colW << 150 << 150;
    row = 0;
    for (int i = 0; i < colW.count(); i++)
        ui->tblGoodsGroup->setColumnWidth(i, colW[i]);
    for (QMultiMap<QString, QString>::const_iterator it = m_goodsGroups.begin(); it != m_goodsGroups.end(); it++) {
        ui->tblGoodsGroup->setItem(row, 0, new QTableWidgetItem(it.key()));
        ui->tblGoodsGroup->setItem(row, 1, new QTableWidgetItem(it.value()));
        row++;
    }
}

void Dialog::makeComboTax()
{
    ui->cbDept->clear();
    for (int i = 0; i < ui->tblTax->rowCount(); i++)
        ui->cbDept->addItem(ui->tblTax->item(i, 2)->data(Qt::DisplayRole).toString(), ui->tblTax->item(i, 0)->data(Qt::DisplayRole).toInt());
    ui->cbDept->setCurrentIndex(0);
}

void Dialog::log(const QString &message)
{
    ui->pteLog->setPlainText(ui->pteLog->document()->toPlainText() + "\n" + QDateTime::currentDateTime().toString() + ": " + message);
}

int Dialog::addRow(QTableWidget *t, const QStringList &data)
{
    int row = t->rowCount();
    t->setRowCount(row + 1);
    for (int i = 0; i < t->columnCount(); i++)
        t->setItem(row, i, new QTableWidgetItem(data[i]));
    return row;
}

bool Dialog::removeRow(QTableWidget *t, QVariant &f)
{
    QModelIndexList m = t->selectionModel()->selectedRows();
    if (!m.count())
        return false;
    if (QMessageBox::warning(this, tr("Warning"), tr("Confirm deletion"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return false;
    f = t->item(m.at(0).row(), 0)->data(Qt::DisplayRole);
    t->removeRow(m.at(0).row());
    return true;
}

bool Dialog::rowData(QTableWidget *t, QStringList &l)
{
    QModelIndexList m = t->selectionModel()->selectedRows();
    if (!m.count())
        return false;
    for (int i = 0; i < t->columnCount(); i++)
        l << t->item(m.at(0).row(), i)->data(Qt::DisplayRole).toString();
    return true;
}

void Dialog::setRowData(QTableWidget *t, QStringList &l)
{
    QModelIndexList m = t->selectionModel()->selectedRows();
    if (!m.count())
        return;
    for (int i = 0; i < l.count(); i++)
        t->item(m.at(0).row(), i)->setData(Qt::DisplayRole, l.at(i));
}

void Dialog::on_btnStart_clicked()
{
    bool active = m_timer.isActive();
    if (active)
    {
        m_timer.stop();
        ui->btnStart->setText(tr("Start"));
    } else {
        m_timer.start(10000);
        ui->btnStart->setText(tr("Stop"));
    }
}

void Dialog::on_btnSave_clicked()
{
    saveSettings();
}

void Dialog::trayActivated(QSystemTrayIcon::ActivationReason a)
{
    switch (a)
    {
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
    case QSystemTrayIcon::Context:
        m_tray->contextMenu()->popup(QCursor::pos());
        break;
    default:
        break;
    }
}

void Dialog::closeEvent(QCloseEvent *e)
{
    if (QMessageBox::question(this, tr("Question"), tr("Are you sure to close?")) != QMessageBox::Yes) {
        e->ignore();
        return;
    }
    QDialog::closeEvent(e);
}

bool Dialog::isWork()
{
    return true;
    /* TODO Remove date limit */

    QDate dateLimit = QDate::fromString("10.03.2028", "dd.MM.yyyy");
    if (QDate::currentDate() > dateLimit)
        qApp->quit();
    return true;
}

void Dialog::on_btnAddTaxConverter_clicked()
{
    QString tax, program, name;
    if (!dlgHdmMap::TaxMap(tax, program, name))
        return;

    QStringList d;
    d << tax << program << name;
    m_taxMap[program] = tax;
    addRow(ui->tblTax, d);
    saveSettings();
    makeComboTax();
}

void Dialog::on_actionQuit_triggered()
{
    qApp->quit();
}

void Dialog::on_btnAddFood_clicked()
{
    m_timer.stop();
    bool active = m_timer.isActive();
    DEdit *d = new DEdit(this);
    d->setCaptions(tr("Fox code"), tr("Arm code"), tr("Name"), tr("Price"), tr("ADG code"));
    if (d->exec() == QDialog::Accepted)
    {
        QString v1, v2, v3, v4, v5;
        d->getData(v1, v2, v3, v4, v5);
        if (!m_fb.open()) {
            log(m_fb.lastError().databaseText());
            if (active)
                m_timer.start();
            return;
        }
        QSqlQuery q(m_fb);
        q.prepare("insert into convert values (:hotel, :armsoft, :name, :price, :adgt);");
        q.bindValue(":hotel", v1);
        q.bindValue(":armsoft", v2);
        q.bindValue(":name", v3);
        q.bindValue(":price", v4.toDouble());
        q.bindValue(":adgt", v5);
        if (!q.exec()) {
            if (active)
                m_timer.start();
            log(q.lastError().databaseText());
            m_fb.close();
            return;
        }
        m_fb.open();
        QStringList d;
        d << v1 << v2 << v3 << v4 << v5;
        addRow(ui->tblGoodsMap, d);
    }
    delete d;
}

void Dialog::on_btnRemoveFood_clicked()
{
    QVariant f;
    if (removeRow(ui->tblGoodsMap, f)) {
        m_fb.open();
        QSqlQuery q(m_fb);
        if (!q.prepare("delete from convert where fox_id=:fox_id")) {
            log(q.lastError().databaseText());
            m_fb.close();
            return;
        }
        q.bindValue(":fox_id", f);
        q.exec();
        m_fb.close();
    }
}

void Dialog::on_btnRemoveTax_clicked()
{
    QVariant f;
    if (removeRow(ui->tblTax, f)) {
        saveSettings();
        makeComboTax();
    }
}

void Dialog::on_btnEditTax_clicked()
{
    QStringList l;
    if (!rowData(ui->tblTax, l))
        return;
    QString v1, v2, v3, temp, v4, v5;
    v1 = l.at(0);
    v2 = l.at(1);
    v3 = l.at(2);
    temp = v2;
    DEdit *d = new DEdit(this);
    d->setData(v1, v2, v3, v4, v5);
    if (d->exec() == QDialog::Accepted) {
        d->getData(v1, v2, v3, v4, v5);
        l.clear();
        l << v1 << v2 << v3;
        setRowData(ui->tblTax, l);
        m_taxMap[v1] = v2;
        saveSettings();
        makeComboTax();
    }
    delete d;
}

void Dialog::on_btnEditFood_clicked()
{
    QStringList l;
    if (!rowData(ui->tblGoodsMap, l))
        return;
    QString v1, v2, v3, v4, v5;
    v1 = l.at(0);
    v2 = l.at(1);
    v3 = l.at(2);
    v4 = l.at(3);
    v5 = l.at(4);
    DEdit *d = new DEdit(this);
    d->setCaptions(tr("Fox code"), tr("Arm code"), tr("Name"), tr("Price"), tr("ADG code"));
    d->setData(v1, v2, v3, v4, v5);
    if (d->exec() == QDialog::Accepted) {
        d->getData(v1, v2, v3, v4, v5);
        l.clear();
        l << v1 << v2 << v3 << v4 << v5;
        setRowData(ui->tblGoodsMap, l);
        if (!m_fb.open()) {
            log(m_fb.lastError().databaseText());
            return;
        }
        QSqlQuery q(m_fb);
        if(!q.prepare("update convert set fox_id=:fox_id, armsoft_id=:armsoft_id, name_arm=:name_arm, price=:price, adgt=:adgt where fox_id=:id"))
            log(q.lastError().databaseText());
        q.bindValue(":fox_id", v1);
        q.bindValue(":armsoft_id", v2);
        q.bindValue(":name_arm", v3);
        q.bindValue(":price", v4.toDouble());
        q.bindValue(":adgt", v5);
        q.bindValue(":id", v1);
        if (!q.exec())
            log(q.lastError().databaseText());
        m_fb.close();
    }
    delete d;
}

void Dialog::on_btnReload_clicked()
{
    reloadMaps();
}

void Dialog::on_btnReload2_clicked()
{
    reloadMaps();
}

void Dialog::on_btnAddGoodsGrouping_clicked()
{
    m_timer.stop();
    bool active = m_timer.isActive();
    DEdit *d = new DEdit(this);
    if (d->exec() == QDialog::Accepted)
    {
        QString v1, v2, v3, v4, v5;
        d->getData(v1, v2, v3, v4, v5);
        if (!m_fb.open()) {
            log(m_fb.lastError().databaseText());
            if (active)
                m_timer.start();
            return;
        }
        QSqlQuery q(m_fb);
        q.prepare("insert into goods_grouping values (:src, :dst);");
        q.bindValue(":src", v1);
        q.bindValue(":dst", v2);
        if (!q.exec()) {
            if (active)
                m_timer.start();
            log(q.lastError().databaseText());
            m_fb.close();
            return;
        }
        m_fb.open();
        QStringList d;
        d << v1 << v2 ;
        addRow(ui->tblGoodsGroup, d);
    }
    delete d;
}

void Dialog::on_btnEditGoodsGrouping_clicked()
{
    QStringList l;
    if (!rowData(ui->tblGoodsGroup, l))
        return;
    QString v1, v2, v3, temp1, temp2, v4, v5;
    v1 = l.at(0);
    v2 = l.at(1);
    temp1 = v1;
    temp2 = v2;
    DEdit *d = new DEdit(this);
    d->setData(v1, v2, v3, v4, v5);
    if (d->exec() == QDialog::Accepted) {
        d->getData(v1, v2, v3, v4, v5);
        l.clear();
        l << v1 << v2;
        setRowData(ui->tblGoodsGroup, l);
        if (!m_fb.open()) {
            log(m_fb.lastError().databaseText());
            return;
        }
        QSqlQuery q(m_fb);
        if (!q.prepare("update goods_grouping set src=:src, dst=:dst where src=:src1 and dst=:dst1")) {
            log("Sql error: " + q.lastError().databaseText());
            m_fb.close();
            return;
        }
        q.bindValue(":src", v1);
        q.bindValue(":dst", v2);
        q.bindValue(":src1", temp1);
        q.bindValue(":dst1", temp2);
        if (!q.exec()) {
            log("Sql error: " + q.lastError().databaseText());
            m_fb.close();
            return;
        }
        m_fb.close();
    }
    delete d;
}

void Dialog::on_btnDelGoodsGrouping_clicked()
{
    QStringList l;
    if (!rowData(ui->tblGoodsGroup, l))
        return;
    if (QMessageBox::warning(this, tr("Confirm deletion"), tr("Confirm deletion"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    if (!m_fb.open()) {
        log(m_fb.lastError().databaseText());
        return;
    }
    QSqlQuery q(m_fb);
    if (!q.prepare("delete from goods_grouping where src=:src and dst=:dst")) {
        log("Sql error: " + q.lastError().databaseText());
        m_fb.close();
        return;
    }
    q.bindValue(":src", l.at(0));
    q.bindValue(":dst", l.at(1));
    if (!q.exec()) {
        log("Sql error: " + q.lastError().databaseText());
        m_fb.close();
        return;
    }
    m_fb.close();
    QModelIndexList m = ui->tblGoodsGroup->selectionModel()->selectedRows();
    if (!m.count())
        return;
    ui->tblGoodsGroup->removeRow(m.at(0).row());
}

void Dialog::on_chAutoPrint_clicked(bool checked)
{
    ui->lePrintTimeout->setEnabled(!checked);
}

void Dialog::on_btnOpenTaxlog_clicked()
{
    QStringList args;
    args << qApp->applicationDirPath() + "/log.txt";
    QProcess::startDetached("notepad.exe", args);
}

void Dialog::on_btnPrintPrepayment_clicked()
{
    isWork();
    QString fileName = "tax_prepayment.json";
    QFile f(m_appPath + "\\" + fileName);
    if (f.open(QIODevice::WriteOnly)) {
        f.write((QString("{\"seq\":1, "
                         "\"paidAmount\":%1, \"paidAmountCard\":%2,"
                         "\"mode\":3, \"useExtPOS\":true,}")
                 .arg(QString::number(ui->leAmountPrepayment->text().toFloat(), 'f', 2))
                 .arg(QString::number(ui->lePrepaymentCard->text().toFloat(), 'f', 2))).toUtf8());
        f.close();
    }

    QStringList args;
    args << ui->leTaxIP->text()
         << ui->leTaxPort->text()
         << ui->leTaxPassword->text()
         << m_appPath;
    QProcess *p = new QProcess();
    p->start(m_appPath + "\\printtax.exe", args);
    ui->leAmountPrepayment->clear();
    ui->lePrepaymentCard->clear();
}

void Dialog::on_btnAddGoods_clicked()
{
    if (ui->leCode->text().isEmpty()) {
        ui->leCode->setFocus();
        return;
    }
    if (ui->leQty->text().toDouble() < 0.01) {
        ui->leQty->setFocus();
        return;
    }
    int row = ui->tblGoodsList->rowCount();
    ui->tblGoodsList->setRowCount(row + 1);
    for (int i = 0; i < ui->tblGoodsList->columnCount(); i++)
        ui->tblGoodsList->setItem(row, i, new QTableWidgetItem());
    ui->tblGoodsList->item(row, 0)->setData(Qt::EditRole, ui->leCode->text());
    ui->tblGoodsList->item(row, 1)->setData(Qt::EditRole, ui->leName->text());
    ui->tblGoodsList->item(row, 2)->setData(Qt::EditRole, ui->leQty->text());
    ui->tblGoodsList->item(row, 3)->setData(Qt::EditRole, ui->lePrice->text());
    ui->tblGoodsList->item(row, 4)->setData(Qt::EditRole, QString::number(ui->leQty->text().toFloat() * ui->lePrice->text().toFloat(), 'f', 3));
    ui->tblGoodsList->item(row, 5)->setData(Qt::EditRole, ui->leADGCode->text());
    ui->leCode->clear();
    ui->leName->clear();
    ui->leQty->clear();
    ui->lePrice->clear();
    ui->leCode->setFocus();
    float total = 0;
    for (int i = 0; i < ui->tblGoodsList->rowCount(); i++)
        total += ui->tblGoodsList->item(i, 4)->data(Qt::EditRole).toFloat();
    ui->leTotal->setText(QString::number(total, 'f', 2));
}

void Dialog::on_btnPrintTaxCheck_clicked()
{
    isWork();
    if (ui->tblGoodsList->rowCount() == 0) {
        return;
    }
    float total = ui->leTotal->text().toFloat();
    if (total < 0.01) {
        QMessageBox::critical(this, tr("Error"), tr("Tax check is incomplete"));
        return;
    }
    float card = 0;
    float prepayment = 0;
    DlgPayment *dl = new DlgPayment(total, this);
    dl->m_timeLeft = 10;
    dl->m_timer.start(1000);
    if (dl->exec() != QDialog::Accepted) {
        return;
        delete dl;
    }
    dl->getCreditAmount(total, card, prepayment);
    delete dl;
    QString fileName = "tax_manually.json";
    QFile f(m_appPath + "\\" + fileName);
    if (f.open(QIODevice::WriteOnly)) {
        f.write((QString("{\"seq\":1, "
                         "\"paidAmount\":%1, \"paidAmountCard\":%2, \"partialAmount\":0, \"prePaymentAmount\":%3,"
                         "\"mode\":2, \"useExtPOS\":false, \"items\":[")
                 .arg(QString::number(total, 'f', 2))
                 .arg(QString::number(card, 'f', 2))
                 .arg(QString::number(prepayment, 'f', 2))).toUtf8());
        bool first = true;
        for (int i = 0; i < ui->tblGoodsList->rowCount(); i++) {
            if (!first)
                f.write(",");
            else
                first = false;
            f.write(QString("{\"dep\":%1,"
                            "\"qty\":%2,"
                            "\"price\":%3,"
                            "\"productCode\":\"%4\","
                            "\"productName\":\"%5\","
                            "\"unit\":\"%6\","
                            "\"totalPrice\":%7,"
                            "\"adgCode\":\"%8\"}")
                    .arg(ui->cbDept->currentData().toInt())
                    .arg(ui->tblGoodsList->item(i, 2)->data(Qt::EditRole).toString())
                    .arg(ui->tblGoodsList->item(i, 3)->data(Qt::EditRole).toString())
                    .arg(ui->tblGoodsList->item(i, 0)->data(Qt::EditRole).toString())
                    .arg(ui->tblGoodsList->item(i, 1)->data(Qt::EditRole).toString())
                    .arg(QString::fromUtf8("հատ"))
                    .arg(ui->tblGoodsList->item(i, 4)->data(Qt::EditRole).toString())
                    .arg(ui->tblGoodsList->item(i, 5)->data(Qt::EditRole).toString()).toUtf8());
        }
        f.write("]}");
        f.close();
    }

    QStringList args;
    args << ui->leTaxIP->text()
         << ui->leTaxPort->text()
         << ui->leTaxPassword->text()
         << m_appPath;
    QProcess *p = new QProcess();
    p->start(m_appPath + "\\printtax.exe", args);

    if (ui->chClearCheckAfterPrint->isChecked()) {
        clearTaxCheck();
    }
    ui->leCode->setFocus();
}

void Dialog::on_btnRemoveRow_clicked()
{
    ui->tblGoodsList->removeRow(ui->tblGoodsList->currentRow());
    float total = 0;
    for (int i = 0; i < ui->tblGoodsList->rowCount(); i++)
        total += ui->tblGoodsList->item(i, 4)->data(Qt::EditRole).toFloat();
    ui->leTotal->setText(QString::number(total, 'f', 2));
}

void Dialog::on_leCode_returnPressed()
{
    if (ui->leCode->text().isEmpty()) {
        return;
    }
    if (m_goodsMap.contains(ui->leCode->text())) {
        ui->leName->setText(m_goodsMap[ui->leCode->text()].armName.trimmed());
        ui->lePrice->setText(QString::number(m_goodsMap[ui->leCode->text()].price, 'f', 0));
        ui->leADGCode->setText(m_goodsMap[ui->leCode->text()].adgt);
    } else {
        ui->leCode->clear();
        return;
    }
    ui->leQty->setFocus();
}

void Dialog::clearTaxCheck()
{
    ui->tblGoodsList->clearContents();
    ui->tblGoodsList->setRowCount(0);
    ui->leCode->setFocus();
}

void Dialog::on_btnClearTaxCheck_clicked()
{
    if (QMessageBox::question(this, tr("Question"), tr("Are you sure to clear tax check?")) != QMessageBox::Yes)
        return;
    clearTaxCheck();
}

void Dialog::on_leQty_returnPressed()
{
    ui->lePrice->setFocus();
}

void Dialog::on_lePrice_returnPressed()
{
    ui->btnAddGoods->setFocus();
}
