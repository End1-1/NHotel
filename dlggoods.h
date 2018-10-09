#ifndef DLGGOODS_H
#define DLGGOODS_H

#include <QDialog>

namespace Ui {
class DlgGoods;
}

class DlgGoods : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGoods(QWidget *parent = 0);
    ~DlgGoods();

private:
    Ui::DlgGoods *ui;
};

#endif // DLGGOODS_H
