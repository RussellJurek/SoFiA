######################################################################
# Created by hand from a qmake template
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT += xml

# Input
HEADERS   += HelpBrowser.h \
             WidgetSpreadsheet.h \
             SoFiA.h
SOURCES   += HelpBrowser.cpp \
             WidgetSpreadsheet.cpp \
             SoFiA.cpp \
             main.cpp
RESOURCES  = SoFiA.qrc
