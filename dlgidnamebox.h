#ifndef DLGIDNAMEBOX_H
#define DLGIDNAMEBOX_H

#include <QDialog>
#include <QLineEdit>
#include <QKeyEvent>

namespace Ui {
class DlgIdNameBox;
}

class DlgIdNameBox : public QDialog
{
    Q_OBJECT

public:
    explicit DlgIdNameBox(QWidget *parent = 0);
    void setFilter(const QStringList &code, const QStringList &name);
    void setName(const QString &name);
    void setCode(const QString &id);
    QStringList m_result;
    ~DlgIdNameBox();
    void accept();
    void keyPressEvent(QKeyEvent *e);
private slots:
    void on_leName_textChanged(const QString &arg1);
    void on_leCode_textChanged(const QString &arg1);
    void on_tblData_doubleClicked(const QModelIndex &index);

private:
    Ui::DlgIdNameBox *ui;
    void setFilterString(const QString &str, QLineEdit *w);
    void filter();
    QMap<QString, QString> m_values;
};

#endif // DLGIDNAMEBOX_H
