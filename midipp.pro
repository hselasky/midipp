#
# QMAKE project file for MIDI Player PRO
#
TEMPLATE	= app
!ios {
CONFIG		+= qt release
}
QT		+= core gui network

# Automatic platform detection
macx {
HAVE_MACOSX=YES
}
ios {
HAVE_IOS=YES
}
android {
HAVE_ANDROID=YES
}
win32 {
HAVE_WIN32=YES
}

greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
    isEmpty(HAVE_IOS) {
        isEmpty(HAVE_ANDROID) {
                QT += printsupport
                DEFINES += HAVE_PRINTER
        }
    }
}

!isEmpty(HAVE_ANDROID) {
HAVE_STATIC=YES
HAVE_NO_SHOW=YES
HAVE_ANDROID=YES
QT += androidextras
QT += gui-private
ANDROID_TARGET_SDK_VERSION=30

DISTFILES+= \
        android/AndroidManifest.xml \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/gradlew \
        android/res/values/libs.xml \
        android/build.gradle \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew.bat \
        android/libs/umidi20_android.jar

ANDROID_PACKAGE_SOURCE_DIR= $${PWD}/android
}

!isEmpty(HAVE_IOS) {
HAVE_STATIC=YES
HAVE_COREMIDI=YES
HAVE_NO_SHOW=YES
SOURCES += ios/activity.mm
HEADERS += ios/activity.h
INCLUDEPATH += ios
QMAKE_ASSET_CATALOGS += ios/Assets.xcassets
QMAKE_INFO_PLIST= ios/midipp.plist
QMAKE_APPLE_DEVICE_ARCHS= arm64
QMAKE_IOS_DEPLOYMENT_TARGET= 12.0
Q_ENABLE_BITCODE.name = ENABLE_BITCODE
Q_ENABLE_BITCODE.value = NO
QMAKE_MAC_XCODE_SETTINGS += Q_ENABLE_BITCODE
}

!isEmpty(HAVE_MACOSX) {
HAVE_STATIC=YES
HAVE_COREMIDI=YES
SOURCES += mac/activity.mm
HEADERS += mac/activity.h
INCLUDEPATH += mac
icons.path	= $${DESTDIR}/Contents/Resources
icons.files	= mac/MidiPlayerPro.icns
QMAKE_BUNDLE_DATA += icons
QMAKE_INFO_PLIST= mac/midipp.plist
OTHER_FILES += mac/MidiPlayerPro.entitlements
}

!isEmpty(HAVE_WIN32) {
HAVE_STATIC=YES
HAVE_JACK=YES
CONFIG  += staticlib
INCLUDEPATH	+= "C:\Program Files\JACK2\include" windows/include
LIBS            += -L"C:\Program Files\JACK2\lib" -ljack64
RC_FILE		= windows/mainicon.rc
}

isEmpty(LIBUMIDIPATH) {
LIBUMIDIPATH=libumidi
}

!isEmpty(HAVE_NO_SHOW) {
DEFINES += HAVE_NO_SHOW
}

!isEmpty(HAVE_SCREENSHOT) {
DEFINES += HAVE_SCREENSHOT
}

HEADERS		+= src/midipp.h
HEADERS		+= src/midipp_bpm.h
HEADERS		+= src/midipp_button.h
HEADERS		+= src/midipp_buttonmap.h
HEADERS		+= src/midipp_chansel.h
HEADERS		+= src/midipp_checkbox.h
HEADERS		+= src/midipp_chords.h
HEADERS		+= src/midipp_custom.h
HEADERS		+= src/midipp_database.h
HEADERS		+= src/midipp_decode.h
HEADERS		+= src/midipp_devices.h
HEADERS		+= src/midipp_devsel.h
HEADERS		+= src/midipp_element.h
HEADERS		+= src/midipp_gpro.h
HEADERS		+= src/midipp_groupbox.h
HEADERS		+= src/midipp_gridlayout.h
HEADERS		+= src/midipp_import.h
HEADERS		+= src/midipp_instrument.h
HEADERS		+= src/midipp_looptab.h
HEADERS		+= src/midipp_mainwindow.h
HEADERS		+= src/midipp_metronome.h
HEADERS		+= src/midipp_midi.h
HEADERS		+= src/midipp_mode.h
HEADERS		+= src/midipp_musicxml.h
HEADERS		+= src/midipp_mutemap.h
HEADERS		+= src/midipp_onlinetabs.h
HEADERS		+= src/midipp_pianotab.h
HEADERS		+= src/midipp_replace.h
HEADERS		+= src/midipp_replay.h
HEADERS		+= src/midipp_scores.h
HEADERS		+= src/midipp_settings.h
HEADERS		+= src/midipp_sheet.h
isEmpty(HAVE_NO_SHOW) {
HEADERS		+= src/midipp_show.h
}
HEADERS		+= src/midipp_spinbox.h
HEADERS		+= src/midipp_shortcut.h
HEADERS		+= src/midipp_tabbar.h
HEADERS		+= src/midipp_volume.h

SOURCES		+= src/midipp.cpp
SOURCES		+= src/midipp_bpm.cpp
SOURCES		+= src/midipp_button.cpp
SOURCES		+= src/midipp_buttonmap.cpp
SOURCES		+= src/midipp_chansel.cpp
SOURCES		+= src/midipp_checkbox.cpp
SOURCES		+= src/midipp_chords.cpp
SOURCES		+= src/midipp_custom.cpp
SOURCES		+= src/midipp_database.cpp
SOURCES		+= src/midipp_decode.cpp
SOURCES		+= src/midipp_devices.cpp
SOURCES		+= src/midipp_devsel.cpp
SOURCES		+= src/midipp_element.cpp
SOURCES		+= src/midipp_gpro.cpp
SOURCES		+= src/midipp_groupbox.cpp
SOURCES		+= src/midipp_gridlayout.cpp
SOURCES		+= src/midipp_import.cpp
SOURCES		+= src/midipp_instrument.cpp
SOURCES		+= src/midipp_looptab.cpp
SOURCES		+= src/midipp_mainwindow.cpp
SOURCES		+= src/midipp_metronome.cpp
SOURCES		+= src/midipp_midi.cpp
SOURCES		+= src/midipp_mode.cpp
SOURCES		+= src/midipp_musicxml.cpp
SOURCES		+= src/midipp_mutemap.cpp
SOURCES		+= src/midipp_onlinetabs.cpp
SOURCES		+= src/midipp_pianotab.cpp
SOURCES		+= src/midipp_replace.cpp
SOURCES		+= src/midipp_replay.cpp
SOURCES		+= src/midipp_scores.cpp
SOURCES		+= src/midipp_settings.cpp
SOURCES		+= src/midipp_sheet.cpp
isEmpty(HAVE_NO_SHOW) {
SOURCES		+= src/midipp_show.cpp
}
SOURCES		+= src/midipp_tabbar.cpp
SOURCES		+= src/midipp_spinbox.cpp
SOURCES		+= src/midipp_shortcut.cpp
SOURCES		+= src/midipp_volume.cpp

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
SOURCES		+= $${LIBUMIDIPATH}/umidi20_pipe.c
INCLUDEPATH	+= $${LIBUMIDIPATH}
 isEmpty(HAVE_CDEV) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_cdev_dummy.c
 } else {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_cdev.c
 }
 isEmpty(HAVE_ALSA) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_alsa_dummy.c
 } else {
 LIBS		+= -L${PREFIX}/lib -lasound
 INCLUDEPATH	+= -I${PREFIX}/include
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_alsa.c
 }
 isEmpty(HAVE_JACK) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_jack_dummy.c
 } else {
 isEmpty(HAVE_WIN32) {
 LIBS		+= -L${PREFIX}/lib -ljack
 INCLUDEPATH	+= -I${PREFIX}/include
 }
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_jack.c
 }
 isEmpty(HAVE_COREMIDI) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_coremidi_dummy.c
 } else {
 LIBS		+= -framework CoreMIDI
 LIBS		+= -framework CoreFoundation
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_coremidi.c
 }
 isEmpty(HAVE_ANDROID) {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_android_dummy.c
 } else {
 SOURCES	+= $${LIBUMIDIPATH}/umidi20_android.c
 OTHER_FILES	+= $${LIBUMIDIPATH}/umidi20_android.java
 }
}

target.path	= $${PREFIX}/bin
INSTALLS	+= target

!macx:!android:!ios:!win32:unix {
icons.path	= $${PREFIX}/share/pixmaps
icons.files	= midipp.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= midipp.desktop
INSTALLS	+= desktop
}
