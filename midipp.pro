TEMPLATE	= app
CONFIG		+= qt warn_on release
QT		+= core gui network
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
  isEmpty(HAVE_IOS) {
    QT += printsupport
  }
}

!isEmpty(HAVE_IOS) {
HAVE_STATIC=YES
HAVE_COREMIDI=YES
HAVE_BUNDLE_ICONS=YES
HAVE_NO_SHOW=YES
icons.path	= $${PREFIX}
icons.files	= midipp_ios.png midipp_ios_retina.png midipp_76x76.png midipp_152x152.png
QMAKE_BUNDLE_DATA += icons
QMAKE_INFO_PLIST= midipp_ios.plist
}

!isEmpty(HAVE_MACOSX) {
HAVE_STATIC=YES
HAVE_COREMIDI=YES
HAVE_BUNDLE_ICONS=YES
icons.path	= $${DESTDIR}/Contents/Resources
icons.files	= MidiPlayerPro.icns
QMAKE_BUNDLE_DATA += icons
QMAKE_INFO_PLIST= midipp_osx.plist
OTHER_FILES += MidiPlayerPro.entitlements
}

isEmpty(LIBUMIDIPATH) {
LIBUMIDIPATH=../libumidi20
}

!isEmpty(HAVE_NO_SHOW) {
DEFINES += HAVE_NO_SHOW
}

!isEmpty(HAVE_SCREENSHOT) {
DEFINES += HAVE_SCREENSHOT
}

HEADERS		+= midipp.h
HEADERS		+= midipp_bpm.h
HEADERS		+= midipp_button.h
HEADERS		+= midipp_buttonmap.h
HEADERS		+= midipp_chansel.h
HEADERS		+= midipp_checkbox.h
HEADERS		+= midipp_custom.h
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
HEADERS		+= midipp_metronome.h
HEADERS		+= midipp_midi.h
HEADERS		+= midipp_mode.h
HEADERS		+= midipp_mutemap.h
HEADERS		+= midipp_pianotab.h
HEADERS		+= midipp_replace.h
HEADERS		+= midipp_replay.h
HEADERS		+= midipp_scores.h
HEADERS		+= midipp_settings.h
isEmpty(HAVE_NO_SHOW) {
HEADERS		+= midipp_show.h
}
HEADERS		+= midipp_spinbox.h
HEADERS		+= midipp_shortcut.h
HEADERS		+= midipp_tabbar.h
HEADERS		+= midipp_volume.h
SOURCES		+= midipp.cpp
SOURCES		+= midipp_bpm.cpp
SOURCES		+= midipp_button.cpp
SOURCES		+= midipp_buttonmap.cpp
SOURCES		+= midipp_chansel.cpp
SOURCES		+= midipp_checkbox.cpp
SOURCES		+= midipp_custom.cpp
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
SOURCES		+= midipp_metronome.cpp
SOURCES		+= midipp_midi.cpp
SOURCES		+= midipp_mode.cpp
SOURCES		+= midipp_mutemap.cpp
SOURCES		+= midipp_pianotab.cpp
SOURCES		+= midipp_replace.cpp
SOURCES		+= midipp_replay.cpp
SOURCES		+= midipp_scores.cpp
SOURCES		+= midipp_settings.cpp
isEmpty(HAVE_NO_SHOW) {
SOURCES		+= midipp_show.cpp
}
SOURCES		+= midipp_tabbar.cpp
SOURCES		+= midipp_spinbox.cpp
SOURCES		+= midipp_shortcut.cpp
SOURCES		+= midipp_volume.cpp

RESOURCES	+= midipp.qrc

isEmpty(HAVE_MACOSX) {
TARGET		= midipp
} else {
TARGET		= MidiPlayerPro
}

LIBS		+= -lz

isEmpty(HAVE_STATIC) {
LIBS		+= -L$${PREFIX}/lib -lumidi20
INCLUDEPATH	+= $${PREFIX}/include
} else {
SOURCES		+= $${LIBUMIDIPATH}/umidi20.c
SOURCES		+= $${LIBUMIDIPATH}/umidi20_file.c
SOURCES		+= $${LIBUMIDIPATH}/umidi20_gen.c
INCLUDEPATH	+= $${LIBUMIDIPATH}
 isEmpty(HAVE_JACK) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_jack_dummy.c
 } else {
 LIBS		+= -L${PREFIX}/lib -ljack
 INCLUDEPATH	+= -I${PREFIX}/include
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_jack.c
 }
 isEmpty(HAVE_COREMIDI) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_coremidi_dummy.c
 } else {
 LIBS		+= -framework CoreMIDI
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_coremidi.c
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
