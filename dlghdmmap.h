#ifndef DLGHDMMAP_H
#define DLGHDMMAP_H

#include <QDialog>

namespace Ui {
class dlgHdmMap;
}

class dlgHdmMap : public QDialog
{
    Q_OBJECT

public:
    explicit dlgHdmMap(QWidget *parent = 0);
    ~dlgHdmMap();
    static bool TaxMap(QString &tax, QString &program, QString &name);

private slots:
    void on_btnCancel_clicked();
    void on_btnOK_clicked();

private:
    Ui::dlgHdmMap *ui;
};

#endif // DLGHDMMAP_H
