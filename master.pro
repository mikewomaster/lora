QT += serialport serialbus widgets

TARGET = LoRaUtility
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
    mqtt.cpp \
    logindialog.cpp \
    system.cpp \
    commanhelper.cpp \
    mbus_protocol.cpp \
    mbus_protocol_aux.cpp \
    mbus_serial.cpp \
    coap.cpp \
    netmodel.cpp \
    dlms.cpp \
    dlms_model.cpp \
    sensor.cpp \
    sensormodel.cpp \
    sensor_edit.cpp \
    obisview.cpp \
    obis_edit.cpp \
    rulechain.cpp \
    rulechainmodbus.cpp \
    handelmodebus.cpp \
    rulechainedit.cpp \
    rulemonitor.cpp \
    eventlogmodel.cpp \
    bucket.cpp \
    ymodem.cpp \
    mapmodel.cpp \
    as.cpp


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
    mqtt.h \
    logindialog.h \
    system.h \
    commanhelper.h \
    mbus_protocol.h \
    mbus_protocol_aux.h \
    coap.h \
    netmodel.h \
    dlms.h \
    dlms_model.h \
    sensor.h \
    sensor_edit.h \
    obis_edit.h \
    rulechain.h \
    rulechainmodbus.h \
    rulechainedit.h \
    rulemonitor.h \
    eventlogmodel.h \
    bucket.h \
    ymodem.h \
    mapmodel.h \
    asdialog.h

FORMS    += mainwindow.ui \
         settingsdialog.ui \
    logdialog.ui \
    logindialog.ui \
    system.ui \
    sensor_edit.ui \
    obis_edit.ui \
    rulechainedit.ui \
    asdialog.ui

RESOURCES += \
    master.qrc

RC_ICONS += logo.ico

target.path = E:\qtWoMasterUtility\TestBuild
INSTALLS += target
