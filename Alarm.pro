#-------------------------------------------------
#
# Project created by QtCreator 2011-07-26T08:36:08
#
#-------------------------------------------------

QT       += core gui

TARGET = worldclock
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    alarmlist.cpp \
    filedelegate.cpp \
    newalarm.cpp \
    dialog.cpp \
    checkdelegate.cpp \
    valuebutton.cpp \
    dialog2.cpp \
    alarmbutton.cpp \
    maintdelegate.cpp \
    world.cpp \
    dialog3.cpp \
    alsettings.cpp \
    mdialog.cpp \
    tdialog.cpp \
    home.cpp

HEADERS  += mainwindow.h \
    alarmlist.h \
    filedelegate.h \
    newalarm.h \
    osso-intl.h \
    dialog.h \
    checkdelegate.h \
    valuebutton.h \
    dialog2.h \
    alarmbutton.h \
    maintdelegate.h \
    world.h \
    dialog3.h \
    alsettings.h \
    mdialog.h \
    tdialog.h \
    home.h \
    gconfitem.h

FORMS    += mainwindow.ui \
    alarmlist.ui \
    newalarm.ui \
    dialog.ui \
    dialog2.ui \
    world.ui \
    dialog3.ui \
    alsettings.ui \
    mdialog.ui \
    tdialog.ui \
    home.ui

CONFIG += mobility
MOBILITY = 

CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0 gtk+-2.0 gstreamer-0.10

symbian {
    TARGET.UID3 = 0xeda884b4
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

maemo5 {
    QT += maemo5
    LIBS += -lalarm -lhildon-time-zone-chooser0 -lclockcore0 -lcityinfo0 -lgq-gconf -ldl
}

maemo5 {
    target.path = /opt/Alarm/bin
    INSTALLS += target
}
