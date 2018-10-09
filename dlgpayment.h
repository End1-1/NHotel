#ifndef DLGPAYMENT_H
#define DLGPAYMENT_H

#include <QDialog>
#include <QTimer>
#include <QLineEdit>
#include <QFocusEvent>
#include "datatypes.h"

namespace Ui {
class DlgPayment;
}

class FLineEdit : public QLineEdit {
       Q_OBJECT
public:
    FLineEdit(QWidget *parent = 0);
protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
signals:
    void focus(bool f);
};

class DlgPayment : public QDialog
{
    Q_OBJECT
private:
    QLineEdit *m_currentEdit;
    void count(float &f1, float &f2, float &f3);
    void setValues(float cash, float credit, float prepayment);
    void disconnectAll();
    void connectAll();
    static QStringList m_itemCode;
    static QStringList m_itemName;

public:
    float m_total;
    int m_timeLeft;
    QTimer m_timer;
    void getCreditAmount(float &total, float &card, float &prepaiment);
    explicit DlgPayment(float amount, QWidget *parent = 0);
    void setGoods(QList<Goods> &goods);
    void getGoods(QList<Goods> &goods);
    ~DlgPayment();

private slots:
    void leQtyChanged(const QString &arg1);
    void lePriceChanged(const QString &arg1);
    void leAmountChanged(const QString &arg1);
    void leNameChanged(const QString &arg1);
    void leCodeChanged(const QString &arg1);
    void on_leCreditCard_textChanged(const QString &arg1);
    void on_leCreditCard_returnPressed();
    void timerTimeout();
    void editFocus(bool v);
    void on_pushButton_12_clicked();
    void on_pushButton_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_13_clicked();
    void on_lePrepaiment_textChanged(const QString &arg1);
    void on_btnCancel_clicked();
    void on_lePrepaiment_returnPressed();
    void on_leCash_textChanged(const QString &arg1);
    void on_leCash_returnPressed();
    void on_btnDelRow_clicked();
    void on_tblGoods_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_btnAddRow_clicked();
    void on_tabWidget_currentChanged(int index);

private:
    Ui::DlgPayment *ui;
    bool m_textSelected;
    void text(QObject *o);
    void countAmount();
};

#endif // DLGPAYMENT_H
