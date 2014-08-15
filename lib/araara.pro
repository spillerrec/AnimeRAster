TEMPLATE = app
CONFIG += console
TARGET = araara
QMAKE_CXXFLAGS += -std=c++11
LIBS += -lz -llzma

# dump library
HEADERS += src/dump/DumpPlane.hpp
SOURCES += src/dump/DumpPlane.cpp

win32{
	# libzpaq
	HEADERS += src/zpaq/libzpaq.h
	SOURCES += src/zpaq/libzpaq.cpp
}

#Planes
HEADERS += src/planes/PixelPlane.hpp src/planes/ValuePlane.hpp src/planes/APlane.hpp
SOURCES += src/planes/PixelPlane.cpp src/planes/ValuePlane.cpp

HEADERS += src/AraImage.hpp src/AraFile.hpp src/AraFrame.hpp src/AraFragment.hpp
SOURCES += src/AraImage.cpp src/AraFile.cpp src/AraFrame.cpp src/AraFragment.cpp

HEADERS += src/device.hpp src/Entropy.hpp src/transform.hpp
SOURCES += src/device.cpp src/Entropy.cpp src/transform.cpp src/main.cpp



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