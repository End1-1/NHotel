#ifndef DEDIT_H
#define DEDIT_H

#include <QDialog>

namespace Ui {
class DEdit;
}

class DEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DEdit(QWidget *parent = 0);
    ~DEdit();
    void setCaptions(const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5);
    void setData(const QString &v1, const QString &v2, const QString &v3, const QString &v4, const QString &v5);
    void getData(QString &v1, QString &v2, QString &v3, QString &v4, QString &v5);

private slots:
    void on_btnCancel_clicked();

    void on_btnOK_clicked();

private:
    Ui::DEdit *ui;
};

#endif // DEDIT_H
