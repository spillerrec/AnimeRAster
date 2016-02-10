TEMPLATE = app
CONFIG += console
TARGET = jpgtests
QMAKE_CXXFLAGS += -std=c++14
LIBS += -lz -llzma -lfftw3-3 -ljpeg


# Plane stuff adapted from Overmix
HEADERS += src/planes/FourierPlane.hpp src/planes/PlaneBase.hpp src/Geometry.hpp
SOURCES += src/planes/FourierPlane.cpp

# Gwenview jpeg reader stuff
HEADERS += src/gwenview/iodevicejpegsourcemanager.h
SOURCES += src/gwenview/iodevicejpegsourcemanager.cpp

# Core code
HEADERS += src/JpegImage.hpp src/PlaneExtras.hpp src/Converters.hpp src/Encoders.hpp
SOURCES += src/JpegImage.cpp src/PlaneExtras.cpp src/Converters.cpp src/Encoders.cpp src/main.cpp


# Compression code from main lib
HEADERS += ../lib/src/transform.hpp
SOURCES += ../lib/src/transform.cpp
win32{
	# libzpaq
	HEADERS += ../lib/src/zpaq/libzpaq.h
	SOURCES += ../lib/src/zpaq/libzpaq.cpp
}


# Generate both debug and release on Linux (disabled)
CONFIG += debug_and_release

# Position of binaries and build files
Release:DESTDIR = release
Release:UI_DIR = release/.ui
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.qrc

Debug:DESTDIR = debug
Debug:UI_DIR = debug/.ui
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.qrc