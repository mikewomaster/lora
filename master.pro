QT += serialbus serialport widgets

TARGET = WoMasterIOT
TEMPLATE = app
CONFIG += c++11
CONFIG += resources_big

SOURCES += main.cpp\
        mainwindow.cpp \
        settingsdialog.cpp \
        writeregistermodel.cpp \
    pidtab.cpp \
    nettab.cpp \
    loratab.cpp \
    serialtab.cpp \
    io.cpp \
    lorasingal.cpp \
    logdialog.cpp \
    lorawan.cpp \
    mbusdeviceset.cpp \
    mbusread.cpp \
    mbusscan.cpp \
    nbiot.cpp \
    adjust.cpp \
    mqtt.cpp

HEADERS  += mainwindow.h \
         settingsdialog.h \
        writeregistermodel.h \
    pidtab.h \
    nettab.h \
    logdialog.h \
    lorawan.h \
    mbusdeviceset.h \
    mbusread.h \
    mbusscan.h \
    nbiot.h \
    adjust.h \
    mqtt.h

FORMS    += mainwindow.ui \
         settingsdialog.ui \
    logdialog.ui

RESOURCES += \
    master.qrc

RC_ICONS += logo.ico

target.path = D:\qt_work
INSTALLS += target
