PROJECT = Qt_comport
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += qextserialport threadcomport

FORMS += main_window_form.ui \
	 crc_form.ui

HEADERS += 	mainwindow.h \
		crc_form.h \
		threadcomport/threadcomport.h \
		qextserialport/qextserialbase.h \
		qextserialport/qextserialport.h

SOURCES += 	main.cpp \
		crc_form.cpp \
		mainwindow.cpp \
		threadcomport/threadcomport.cpp \
		qextserialport/qextserialbase.cpp \
		qextserialport/qextserialport.cpp

RESOURCES     = resources.grc

unix:{
	DEFINES   = _TTY_POSIX_
	HEADERS +=  qextserialport/posix_qextserialport.h
	SOURCES +=  qextserialport/posix_qextserialport.cpp
}
win32:{
	DEFINES  = _TTY_WIN_
	HEADERS +=  qextserialport/win_qextserialport.h
	SOURCES +=  qextserialport/win_qextserialport.cpp
}

win32-msvc*:{
RC_FILE = Qt_comport.rc
}

BUILDDIR        = .build/
win32:BUILDDIR  = $$join(BUILDDIR,,,win32)
unix:BUILDDIR   = $$join(BUILDDIR,,,unix)
macx:BUILDDIR   = $$join(BUILDDIR,,,macx)
 
UI_DIR          = $${BUILDDIR}/ui
UIC_DIR         = $${BUILDDIR}/uic
MOC_DIR         = $${BUILDDIR}/moc
RCC_DIR         = $${BUILDDIR}/rcc
OBJECTS_DIR     = $${BUILDDIR}/obj
 
CONFIG(release, debug|release) {
    OBJECTS_DIR	= $$join(OBJECTS_DIR,,,/release)
}
else {
    OBJECTS_DIR	= $$join(OBJECTS_DIR,,,/debug)
}
