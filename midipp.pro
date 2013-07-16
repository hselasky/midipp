TEMPLATE	= app
CONFIG		+= qt warn_on release
QT		+= core gui
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets printsupport
DEFINES		+= HAVE_PRINTSUPPORT
}
HEADERS		+= midipp.h
HEADERS		+= midipp_bpm.h
HEADERS		+= midipp_button.h
HEADERS		+= midipp_buttonmap.h
HEADERS		+= midipp_decode.h
HEADERS		+= midipp_devices.h
HEADERS		+= midipp_gpro.h
HEADERS		+= midipp_import.h
HEADERS		+= midipp_looptab.h
HEADERS		+= midipp_mainwindow.h
HEADERS		+= midipp_midi.h
HEADERS		+= midipp_mode.h
HEADERS		+= midipp_mutemap.h
HEADERS		+= midipp_replace.h
HEADERS		+= midipp_scores.h
HEADERS		+= midipp_settings.h
HEADERS		+= midipp_spinbox.h
HEADERS		+= midipp_pattern.h
HEADERS		+= midipp_volume.h
SOURCES		+= midipp.cpp
SOURCES		+= midipp_bpm.cpp
SOURCES		+= midipp_button.cpp
SOURCES		+= midipp_buttonmap.cpp
SOURCES		+= midipp_decode.cpp
SOURCES		+= midipp_devices.cpp
SOURCES		+= midipp_gpro.cpp
SOURCES		+= midipp_import.cpp
SOURCES		+= midipp_looptab.cpp
SOURCES		+= midipp_mainwindow.cpp
SOURCES		+= midipp_midi.cpp
SOURCES		+= midipp_mode.cpp
SOURCES		+= midipp_mutemap.cpp
SOURCES		+= midipp_replace.cpp
SOURCES		+= midipp_scores.cpp
SOURCES		+= midipp_settings.cpp
SOURCES		+= midipp_pattern.cpp
SOURCES		+= midipp_spinbox.cpp
SOURCES		+= midipp_volume.cpp

RESOURCES	+= midipp.qrc

TARGET		= midipp

isEmpty(STATIC_BUILD) {
LIBS		+= -L$${PREFIX}/lib -lumidi20
INCLUDEPATH	+= $${PREFIX}/include
} else {
SOURCES		+= ../libumidi20/umidi20.c
SOURCES		+= ../libumidi20/umidi20_file.c
SOURCES		+= ../libumidi20/umidi20_gen.c
SOURCES		+= ../libumidi20/umidi20_jack_dummy.c
INCLUDEPATH	+= ../libumidi20
}

target.path	= $${PREFIX}/bin
INSTALLS	+= target

icons.path	= $${PREFIX}/share/pixmaps
icons.files	= midipp.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= midipp.desktop
INSTALLS	+= desktop
