#-------------------------------------------------
#
# Project created by QtCreator 2018-03-02T15:35:20
#
#-------------------------------------------------

QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

macx {
    TARGET = IOTAcooler
}

unix:!macx|win32 {
    TARGET = iota-cooler
}

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


SOURCES += \
        main.cpp \
        widgets/mainwindow.cpp \
    utils/definitionholder.cpp \
    widgets/settingsdialog.cpp \
    components/settingsmanager.cpp \
    utils/qtsingleapplication/qtlocalpeer.cpp \
    utils/qtsingleapplication/qtlockedfile_unix.cpp \
    utils/qtsingleapplication/qtlockedfile_win.cpp \
    utils/qtsingleapplication/qtlockedfile.cpp \
    utils/qtsingleapplication/qtsingleapplication.cpp \
    utils/qtsingleapplication/qtsinglecoreapplication.cpp \
    widgets/aboutdialog.cpp \
    components/abstracttangleapi.cpp \
    components/smidgentangleapi.cpp \
    widgets/promotereattachdialog.cpp \
    utils/utilsiota.cpp \
    widgets/createwalletwizard.cpp \
    utils/Qt-AES/qaesencryption.cpp \
    components/walletmanager.cpp \
    widgets/walletpassphrasedialog.cpp \
    widgets/walletwidget.cpp

HEADERS += \
        widgets/mainwindow.h \
    utils/definitionholder.h \
    widgets/settingsdialog.h \
    components/settingsmanager.h \
    utils/qtsingleapplication/qtlocalpeer.h \
    utils/qtsingleapplication/qtlockedfile.h \
    utils/qtsingleapplication/qtsingleapplication.h \
    utils/qtsingleapplication/qtsinglecoreapplication.h \
    widgets/aboutdialog.h \
    components/abstracttangleapi.h \
    components/smidgentangleapi.h \
    widgets/promotereattachdialog.h \
    utils/utilsiota.h \
    widgets/createwalletwizard.h \
    utils/Qt-AES/qaesencryption.h \
    components/walletmanager.h \
    widgets/walletpassphrasedialog.h \
    widgets/walletwidget.h

FORMS += \
        widgets/mainwindow.ui \
    widgets/settingsdialog.ui \
    widgets/aboutdialog.ui \
    widgets/promotereattachdialog.ui \
    widgets/createwalletwizard.ui \
    widgets/walletpassphrasedialog.ui \
    widgets/walletwidget.ui

ICON = resources/icons/iotacooler.icns # for mac
RC_FILE = resources/iotacooler.rc # for windows

RESOURCES += \
    resources/resources.qrc
