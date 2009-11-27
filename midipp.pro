TEMPLATE        = app
unix:CONFIG     += qt warn_on release
win32:CONFIG    += windows warn_on release
HEADERS         = midipp.h
SOURCES         = midipp.cpp
TARGET          = midipp
QTDIR_build:REQUIRES="contains(QT_CONFIG, full-config)"
unix:LIBS      += -lumidi20
