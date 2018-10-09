#ifndef DATATYPES_H
#define DATATYPES_H

struct Goods {
    QString foxId;
    QString armsoftId;
    QString armName;
    QString adgt;
    float qty;
    float price;
    int dept;
    Goods(){foxId = "", armsoftId = "";}
};

struct accGoods {
    QString name;
    QString salCode;
    QString entDate;
    double qty;
    double amount;
    QString outNo;
    QString jv;
    QString serial;
};

#endif // DATATYPES_H
