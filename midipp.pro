TEMPLATE	= app
CONFIG		+= qt warn_on release
QT		+= core gui network
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets printsupport
}

!isEmpty(HAVE_IOS) {
HAVE_STATIC=YES
HAVE_COREMIDI=YES
HAVE_BUNDLE_ICONS=YES
icons.path	= $${PREFIX}
icons.files	= midipp_ios.png midipp_ios_retina.png
QMAKE_BUNDLE_DATA += icons
}

!isEmpty(HAVE_MACOSX) {
HAVE_STATIC=YES
HAVE_COREMIDI=YES
HAVE_BUNDLE_ICONS=YES
icons.path	= $${PREFIX}
icons.files	= midipp.icns
QMAKE_BUNDLE_DATA += icons
}

HEADERS		+= midipp.h
HEADERS		+= midipp_bpm.h
HEADERS		+= midipp_button.h
HEADERS		+= midipp_buttonmap.h
HEADERS		+= midipp_checkbox.h
HEADERS		+= midipp_database.h
HEADERS		+= midipp_decode.h
HEADERS		+= midipp_devices.h
HEADERS		+= midipp_element.h
HEADERS		+= midipp_gpro.h
HEADERS		+= midipp_groupbox.h
HEADERS		+= midipp_gridlayout.h
HEADERS		+= midipp_import.h
HEADERS		+= midipp_looptab.h
HEADERS		+= midipp_mainwindow.h
HEADERS		+= midipp_midi.h
HEADERS		+= midipp_mode.h
HEADERS		+= midipp_mutemap.h
HEADERS		+= midipp_replace.h
HEADERS		+= midipp_scores.h
HEADERS		+= midipp_settings.h
HEADERS		+= midipp_show.h
HEADERS		+= midipp_spinbox.h
HEADERS		+= midipp_pattern.h
HEADERS		+= midipp_volume.h
SOURCES		+= midipp.cpp
SOURCES		+= midipp_bpm.cpp
SOURCES		+= midipp_button.cpp
SOURCES		+= midipp_buttonmap.cpp
SOURCES		+= midipp_checkbox.cpp
SOURCES		+= midipp_database.cpp
SOURCES		+= midipp_decode.cpp
SOURCES		+= midipp_devices.cpp
SOURCES		+= midipp_element.cpp
SOURCES		+= midipp_gpro.cpp
SOURCES		+= midipp_groupbox.cpp
SOURCES		+= midipp_gridlayout.cpp
SOURCES		+= midipp_import.cpp
SOURCES		+= midipp_looptab.cpp
SOURCES		+= midipp_mainwindow.cpp
SOURCES		+= midipp_midi.cpp
SOURCES		+= midipp_mode.cpp
SOURCES		+= midipp_mutemap.cpp
SOURCES		+= midipp_replace.cpp
SOURCES		+= midipp_scores.cpp
SOURCES		+= midipp_settings.cpp
SOURCES		+= midipp_show.cpp
SOURCES		+= midipp_pattern.cpp
SOURCES		+= midipp_spinbox.cpp
SOURCES		+= midipp_volume.cpp

RESOURCES	+= midipp.qrc

TARGET		= midipp

LIBS		+= -lz

isEmpty(HAVE_STATIC) {
LIBS		+= -L$${PREFIX}/lib -lumidi20
INCLUDEPATH	+= $${PREFIX}/include
} else {
SOURCES		+= ../libumidi20/umidi20.c
SOURCES		+= ../libumidi20/umidi20_file.c
SOURCES		+= ../libumidi20/umidi20_gen.c
INCLUDEPATH	+= ../libumidi20
 isEmpty(HAVE_JACK) {
 SOURCES	+= ../libumidi20/umidi20_jack_dummy.c
 } else {
 LIBS		+= -L${PREFIX}/lib -ljack
 INCLUDEPATH	+= -I${PREFIX}/include
 SOURCES	+= ../libumidi20/umidi20_jack.c
 }
 isEmpty(HAVE_COREMIDI) {
 SOURCES	+= ../libumidi20/umidi20_coremidi_dummy.c
 } else {
 LIBS		+= -framework CoreMIDI
 SOURCES	+= ../libumidi20/umidi20_coremidi.c
 }
}

target.path	= $${PREFIX}/bin
INSTALLS	+= target

isEmpty(HAVE_BUNDLE_ICONS) {
  icons.path	= $${PREFIX}/share/pixmaps
  icons.files	= midipp.png
  INSTALLS	+= icons
}

desktop.path	= $${PREFIX}/share/applications
desktop.files	= midipp.desktop
INSTALLS	+= desktop
