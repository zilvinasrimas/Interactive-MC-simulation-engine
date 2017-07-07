#-------------------------------------------------
#
# Project created by QtCreator 2015-05-25T18:08:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = vpart
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    qcustomplot/qcustomplot.cpp \
    worker.cpp \
    part_mc.cpp \
    helper.cpp \
    mudensdialog.cpp \
    sample3ddialog.cpp \
    mymgldraw.cpp \
    part_mc_base.cpp \
    part_mc_base_y12.cpp


HEADERS  += dialog.h \
    qcustomplot/qcustomplot.h \
    worker.h \
    part_mc.h \
    helper.h \
    mudensdialog.h \
    sample3ddialog.h \
    mymgldraw.h \
    part_mc_base.h \
    part_mc_base_y12.h

FORMS    += dialog.ui \
    mudensdialog.ui \
    sample3ddialog.ui

LIBS += -lmgl -lmgl-qt5

INCLUDEPATH += $$PWD/qcustomplot/

QMAKE_CFLAGS = -O2
QMAKE_CFLAGS -= -O1
QMAKE_CXXFLAGS = -O2
QMAKE_CXXFLAGS -= -O1
QMAKE_CFLAGS = -m64 -O3
QMAKE_LFLAGS = -m64 -O3
QMAKE_CXXFLAGS = -m64 -O3

QMAKE_CXXFLAGS += -std=gnu++11
