TEMPLATE = app
CONFIG += console
TARGET = araara
QMAKE_CXXFLAGS += -std=c++11
LIBS += -lz -llzma

HEADERS += src/dump/DumpPlane.hpp
SOURCES += src/dump/DumpPlane.cpp

HEADERS += src/AraImage.hpp src/transform.hpp
SOURCES += src/AraImage.cpp src/transform.cpp src/main.cpp
