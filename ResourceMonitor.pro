#-------------------------------------------------
#
# Project created by QtCreator 2023-05-08T13:40:55
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ResourceMonitor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        resource_monitor.cpp \
    data_center.cpp \
    pugixml.cpp \
    flowlayout.cpp \
    percentunit.cpp \
    readxmlrunnable.cpp \
    treemodel.cpp \
    callout.cpp \
    MultiSelectComboBox.cpp \
    wcombobox.cpp \
    combobox.cpp

HEADERS += \
        resource_monitor.h \
    data_center.h \
    pugixml.h \
    pugiconfig.hpp \
    wcombobox.h \
    flowlayout.h \
    percentunit.h \
    global.h \
    readxmlrunnable.h \
    treemodel.h \
    callout.h \
    MultiSelectComboBox.h \
    combobox.h

FORMS += \
        resource_monitor.ui \
    percentunit.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

RC_FILE = logo.rc
