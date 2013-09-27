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
    alarmsndpick.cpp \
    home.cpp \
    dialogdate.cpp \
    citydetail.cpp

HEADERS  += mainwindow.h \
    alarmlist.h \
    filedelegate.h \
    ListDelegate.h \
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
    alarmsndpick.h \
    home.h \
    dialogdate.h \
	Metrics.h \
    citydetail.h

FORMS    += mainwindow.ui \
    alarmlist.ui \
    newalarm.ui \
    dialog.ui \
    dialog2.ui \
    world.ui \
    dialog3.ui \
    alsettings.ui \
    mdialog.ui \
    alarmsndpick.ui \
    tdialog.ui \
    home.ui \
    dialogdate.ui \
    citydetail.ui

CONFIG += mobility
MOBILITY = 

CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-0.10 gq-gconf alarm libcityinfo0-0

symbian {
    TARGET.UID3 = 0xeda884b4
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

unix {
   INSTALLS += target desktop
   target.path = $$PREFIX/bin
   desktop.path = $$PREFIX/share/applications/hildon
   desktop.files += worldclock.desktop
}

maemo5 {
    QT += maemo5 dbus
    LIBS += -ldl -ltime
    INSTALLS += service
    service.path = $$PREFIX/share/dbus-1/services
    service.files += com.nokia.worldclock.service
}
