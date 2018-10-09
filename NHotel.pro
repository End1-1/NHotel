#-------------------------------------------------
#
# Project created by QtCreator 2015-06-29T20:12:53
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NHotel
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    dedit.cpp \
    dlghdmmap.cpp \
    dlgpayment.cpp \
    dlgidnamebox.cpp \
    dlggoods.cpp

HEADERS  += dialog.h \
    dedit.h \
    dlghdmmap.h \
    dlgpayment.h \
    datatypes.h \
    dlgidnamebox.h \
    dlggoods.h

FORMS    += dialog.ui \
    dedit.ui \
    dlghdmmap.ui \
    dlgpayment.ui \
    dlgidnamebox.ui \
    dlggoods.ui

ICON = app.ico

RC_FILE = rc.rc

RESOURCES += \
    res.qrc
