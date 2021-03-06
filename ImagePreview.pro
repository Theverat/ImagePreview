#-------------------------------------------------
#
# Project created by QtCreator 2014-06-30T18:53:40
#
#-------------------------------------------------

QT       += core gui \
    printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImagePreview
TEMPLATE = app



SOURCES += main.cpp\
        mainwindow.cpp \
    graphicsscene.cpp \
    graphicsview.cpp \
    imagehandler.cpp \
    convertimagesdialog.cpp \
    trashhandler.cpp \
    trashedfile.cpp \
    restoretrashdialog.cpp \
    cursormanager.cpp \
    helpdialog.cpp \
    exifparser.cpp

HEADERS  += mainwindow.h \
    graphicsscene.h \
    graphicsview.h \
    imagehandler.h \
    convertimagesdialog.h \
    trashhandler.h \
    trashedfile.h \
    restoretrashdialog.h \
    cursormanager.h \
    helpdialog.h \
    exifparser.h

FORMS    += mainwindow.ui \
    convertimagesdialog.ui \
    restoretrashdialog.ui \
    helpdialog.ui
