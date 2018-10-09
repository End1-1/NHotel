#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>
#include <QSqlDatabase>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include <QTableWidget>
#include "datatypes.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_pushButton_clicked();
    void timeout();
    void saveSettings();
    void on_btnStart_clicked();
    void on_btnSave_clicked();
    void trayActivated(QSystemTrayIcon::ActivationReason a);
    void on_btnAddTaxConverter_clicked();
    void on_actionQuit_triggered();
    void on_btnAddFood_clicked();
    void on_btnRemoveFood_clicked();
    void on_btnRemoveTax_clicked();
    void on_btnEditTax_clicked();
    void on_btnEditFood_clicked();
    void on_btnReload_clicked();
    void on_btnReload2_clicked();
    void on_btnAddGoodsGrouping_clicked();
    void on_btnEditGoodsGrouping_clicked();
    void on_btnDelGoodsGrouping_clicked();
    void on_chAutoPrint_clicked(bool checked);
    void on_btnOpenTaxlog_clicked();
    void on_btnPrintPrepayment_clicked();
    void on_btnAddGoods_clicked();
    void on_btnPrintTaxCheck_clicked();
    void on_btnRemoveRow_clicked();
    void on_leCode_returnPressed();
    void on_btnClearTaxCheck_clicked();
    void on_leQty_returnPressed();
    void on_lePrice_returnPressed();

protected:
    void closeEvent(QCloseEvent *e);

private:
    Ui::Dialog *ui;
    QSystemTrayIcon *m_tray;
    QSqlDatabase m_fb;
    QTimer m_timer;
    QString m_appPath;
    QMap<QString, Goods> m_goodsMap;
    QMap<QString, QString> m_taxMap;
    QMultiMap<QString, QString> m_goodsGroups;

    bool isWork();
    void initDb();
    void loadSettings();
    void printTax();
    void reloadMaps();
    void makeComboTax();
    void log(const QString &message);
    int addRow(QTableWidget *t, const QStringList &data);
    bool removeRow(QTableWidget *t, QVariant &f);
    bool rowData(QTableWidget *t, QStringList &l);
    void setRowData(QTableWidget *t, QStringList &l);
    void clearTaxCheck();
};

#endif // DIALOG_H
