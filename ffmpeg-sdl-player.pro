TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lSDL2
LIBS += -lavcodec -lavformat -lswscale -lavutil

SOURCES += main.cpp
